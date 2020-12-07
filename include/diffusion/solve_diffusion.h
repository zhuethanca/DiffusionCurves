#pragma once
#include <Eigen/Eigen>

void solve_diffusion(const Eigen::SparseMatrix<double> &rgb, const Eigen::SparseMatrix<double> &norms,
                     size_t width, size_t height, Eigen::MatrixXd &rgbImage);