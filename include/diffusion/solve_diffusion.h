#pragma once
#include <Eigen/Eigen>

/**
 * Given a set of known points rgb, and the color gradient in norms,
 * compute the sharp rgb image.
 */
void solve_diffusion(const Eigen::SparseMatrix<double> &rgb, const Eigen::SparseMatrix<double> &norms,
                     size_t width, size_t height, Eigen::MatrixXd &rgbImage);