#pragma once
#include <Eigen/Eigen>

void solve_gaussian(const Eigen::SparseMatrix<double> &blur,
                     size_t width, size_t height, Eigen::MatrixXd &blurMap);