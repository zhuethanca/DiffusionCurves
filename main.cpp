#include <cstdlib>
#include <cstdio>
#include <Eigen/Eigen>
#include <iostream>
#include "fd_grad.h"
#include "igl/min_quad_with_fixed.h"
#include "Eigen/SparseCholesky"

#define WIDTH 1920
#define HEIGHT 900

#define INDEX22(i, j) ((i)+(j)*WIDTH)
#define INDEX23(i, j, k) ((i)+(j)*WIDTH+(k)*WIDTH*HEIGHT)

int main(int argc, char *argv[]) {
    FILE* gradientFile = fopen("../data/gradient.bin", "r");
    std::vector<Tripletd> rgbList;

    for (int y = 0; y < HEIGHT; y ++) {
        for (int x = 0; x < WIDTH; x ++) {
            uint32_t argb;
            fread(&argb, sizeof(uint32_t), 1, gradientFile);
            uint8_t a = (argb>>24)&0xFF;
            uint8_t r = (argb>>16)&0xFF;
            uint8_t g = (argb>> 8)&0xFF;
            uint8_t b = (argb>> 0)&0xFF;
            if (a) {
                rgbList.emplace_back(INDEX22(x, y), 0, r);
                rgbList.emplace_back(INDEX22(x, y), 1, g);
                rgbList.emplace_back(INDEX22(x, y), 2, b);
            }
        }
    }
    fclose(gradientFile);

    Eigen::SparseMatrix<double> rgbImg;
    rgbImg.resize(HEIGHT*WIDTH, 3);
    rgbImg.setFromTriplets(rgbList.begin(), rgbList.end());

    Eigen::MatrixX3d rgbDense(rgbImg.nonZeros()/3, 3);
    Eigen::VectorXi known(rgbImg.nonZeros()/3);
    {
        int i = 0;
        for (Eigen::SparseMatrix<double>::InnerIterator it(rgbImg, 0); it; ++it) {
            known(i) = it.row();
            rgbDense(i, 0) = it.value();
            rgbDense(i, 1) = rgbImg.coeff(it.row(), 1);
            rgbDense(i, 2) = rgbImg.coeff(it.row(), 2);
            i++;
        }
    }

    FILE* locFile = fopen("../data/loc.bin", "r");
    FILE* rFile = fopen("../data/red.bin", "r");
    FILE* gFile = fopen("../data/green.bin", "r");
    FILE* bFile = fopen("../data/blue.bin", "r");

    uint32_t N = 0;
    fread(&N, sizeof(uint32_t), 1, locFile);
    Eigen::MatrixXi locs(N, 2);
    Eigen::MatrixXd redVec(N, 2);
    Eigen::MatrixXd greenVec(N, 2);
    Eigen::MatrixXd blueVec(N, 2);

    for (int i = 0; i < N; i ++) {
        uint32_t locx, locy;
        double rx, ry, gx, gy, bx, by;
        fread(&locx, sizeof(uint32_t), 1, locFile);
        fread(&locy, sizeof(uint32_t), 1, locFile);
        fread(&rx, sizeof(double), 1, rFile);
        fread(&ry, sizeof(double), 1, rFile);
        fread(&gx, sizeof(double), 1, gFile);
        fread(&gy, sizeof(double), 1, gFile);
        fread(&bx, sizeof(double), 1, bFile);
        fread(&by, sizeof(double), 1, bFile);
        locs(i, 0) = locx;
        locs(i, 1) = locy;
        redVec(i, 0) = rx;
        redVec(i, 1) = ry;
        greenVec(i, 0) = gx;
        greenVec(i, 1) = gy;
        blueVec(i, 0) = bx;
        blueVec(i, 1) = by;
    }

    fclose(locFile);
    fclose(rFile);
    fclose(gFile);
    fclose(bFile);

    Eigen::SparseMatrix<double> W;
    W.resize(HEIGHT*WIDTH*2, 3);

    std::vector<Tripletd> wList;

    for (int i = 0; i < N; i ++) {
        int locx = locs(i, 0);
        int locy = locs(i, 1);
        wList.emplace_back(INDEX23(locx, locy, 0), 0, redVec(i, 0));
        wList.emplace_back(INDEX23(locx, locy, 1), 0,redVec(i, 1));
        wList.emplace_back(INDEX23(locx, locy, 0), 1,greenVec(i, 0));
        wList.emplace_back(INDEX23(locx, locy, 1), 1,greenVec(i, 1));
        wList.emplace_back(INDEX23(locx, locy, 0), 2,blueVec(i, 0));
        wList.emplace_back(INDEX23(locx, locy, 1), 2,blueVec(i, 1));
    }

    W.setFromTriplets(wList.begin(), wList.end());
//    std::cout << known.rows() << std::endl;
//    return 0;
    int nx = WIDTH;
    int ny = HEIGHT;
    Eigen::SparseMatrix<double> G(nx * ny * 2, nx * ny);
    fd_grad(nx, ny, 2, G);

    Eigen::SparseMatrix<double> A = G.transpose() * G;
    Eigen::SparseMatrix<double> Aeq(0, A.rows());
    igl::min_quad_with_fixed_data<double> data;
    igl::min_quad_with_fixed_precompute(A, known, Aeq, false, data);

    Eigen::SparseMatrix<double> B = G.transpose() * W;
    Eigen::MatrixXd Beq = Eigen::MatrixXd::Zero(0, 0);

    Eigen::MatrixXd finalImage;
    igl::min_quad_with_fixed_solve(data, B.toDense(), rgbDense, Beq, finalImage);

    FILE* res = fopen("../data/res.bin", "wb");
    for (int y = 0; y < HEIGHT; y ++) {
        for (int x = 0; x < WIDTH; x ++) {
            uint8_t a = 0xFF;
            uint8_t r = ((uint32_t) round(finalImage(INDEX22(x, y), 0))) & 0xFF;
            uint8_t g = ((uint32_t) round(finalImage(INDEX22(x, y), 1))) & 0xFF;
            uint8_t b = ((uint32_t) round(finalImage(INDEX22(x, y), 2))) & 0xFF;
            uint32_t argb = (a << 24) | (r << 16) | (g << 8) | b;
            fwrite(&argb, sizeof(uint32_t), 1, res);
        }
    }
    fclose(res);
    return EXIT_SUCCESS;
}
