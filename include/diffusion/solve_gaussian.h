#pragma once
#include <Eigen/Eigen>

/**
 * Given a set of known points blur, compute the blur map.
 */
void solve_gaussian(const Eigen::SparseMatrix<double> &blur,
                     size_t width, size_t height, Eigen::MatrixXd &blurMap);