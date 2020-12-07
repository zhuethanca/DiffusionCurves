#include "diffusion/apply_blur.h"

#include "util/util.h"

void generateGaussian(Eigen::MatrixXd &kernel, int x, int y, int width, int height,
                      double sigma, int *kx, int *ky);

void apply_blur(Eigen::MatrixXd &rgbImage, Eigen::MatrixXd &blurMap,
                size_t width, size_t height, Eigen::MatrixXd &blurredImage) {
    blurredImage.resizeLike(rgbImage);
    std::vector<Tripletd> matList(ceil(blurMap.maxCoeff()*6)*rgbImage.rows());

    for (int x = 0; x < width; x ++) {
        for (int y = 0; y < height; y ++) {
            uint32_t idx = index(x, y, width, height);
            double sigma = blurMap(idx, 0);
            Eigen::MatrixXd kernel;
            int kx, ky;
            generateGaussian(kernel, x, y, width, height, sigma, &kx, &ky);
            for (int rx = 0; rx < kernel.rows(); rx ++) {
                for (int ry = 0; ry < kernel.cols(); ry ++) {
                    matList.emplace_back(index(x, y, width, height), index(kx+rx, ky+ry, width, height), kernel(rx, ry));
                }
            }
        }
    }
    Eigen::SparseMatrix<double, Eigen::RowMajor> kernelMatrix(rgbImage.rows(), rgbImage.rows());
    kernelMatrix.setFromTriplets(matList.begin(), matList.end());
    blurredImage = kernelMatrix*rgbImage;
}

void generateGaussian(Eigen::MatrixXd &kernel, int x, int y, int width, int height,
                      double sigma, int *kx, int *ky) {
    int size = ceil(3*sigma);
    int ls = MIN(size, x);
    int rs = MIN(size, width-x-1);
    int ts = MIN(size, y);
    int bs = MIN(size, height-y-1);
    kernel.resize(ls+rs+1, ts+bs+1);
    *kx = x-ls;
    *ky = y-ts;
    for (int rx = x-ls; rx <= x+rs; rx ++) {
        for (int ry = y-ts; ry <= y + bs; ry ++) {
            kernel(rx-*kx, ry-*ky) = (rx-x)*(rx-x) + (ry-y)*(ry-y);
        }
    }
    kernel /= -2*sigma*sigma;
    kernel.array() = Eigen::exp(kernel.array());
    kernel /= (2*EIGEN_PI*sigma*sigma);
}