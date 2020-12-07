#include "diffusion/solve_diffusion.h"
#include "diffusion/fd_grad.h"

#include <igl/min_quad_with_fixed.h>
#include <set>

void solve_diffusion(const Eigen::SparseMatrix<double> &rgb, const Eigen::SparseMatrix<double> &norms,
                     size_t width, size_t height, Eigen::MatrixXd &rgbImage) {

    std::set<int> known_set;
    for (int k = 0; k < 3; k++) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(rgb, k); it; ++it) {
            known_set.emplace(it.row());
        }
    }
    Eigen::MatrixX3d rgbDense(known_set.size(), 3);
    Eigen::VectorXi known(known_set.size());
    {
        int i = 0;
        for (auto k : known_set) {
            known(i) = k;
            rgbDense(i, 0) = rgb.coeff(k, 0);
            rgbDense(i, 1) = rgb.coeff(k, 1);
            rgbDense(i, 2) = rgb.coeff(k, 2);
            i++;
        }
    }

    int nx = width;
    int ny = height;
    Eigen::SparseMatrix<double> G(nx * ny * 2, nx * ny);
    fd_grad(nx, ny, G);
    // G *= 1.5;

    Eigen::SparseMatrix<double> A = G.transpose() * G;
    Eigen::SparseMatrix<double> Aeq(0, A.rows());

    Eigen::SparseMatrix<double> B = G.transpose() * norms;
    Eigen::MatrixXd Beq = Eigen::MatrixXd::Zero(0, 0);

    igl::min_quad_with_fixed(A, B.toDense(), known, rgbDense, Aeq, Beq, false, rgbImage);

    // Normalize the image into the range 0.0 - 1.0.
    const double min = rgbImage.minCoeff();
    const double max = rgbImage.maxCoeff();

    Eigen::MatrixXd constantTerm;
    constantTerm.resizeLike(rgbImage);
    constantTerm.setConstant(min);

    rgbImage -= constantTerm;
    rgbImage /= (max - min);
}