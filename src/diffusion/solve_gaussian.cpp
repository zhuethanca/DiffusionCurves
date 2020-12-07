#include "diffusion/solve_gaussian.h"
#include "diffusion/fd_grad.h"

#include "util/util.h"

#include <igl/min_quad_with_fixed.h>
#include <set>

void solve_gaussian(const Eigen::SparseMatrix<double> &blur,
                    size_t width, size_t height, Eigen::MatrixXd &blurMap) {
    std::set<int> known_set;
    for (Eigen::SparseMatrix<double>::InnerIterator it(blur, 0); it; ++it) {
        known_set.emplace(it.row());
    }
    Eigen::MatrixXd blurDense(known_set.size(), 1);
    Eigen::VectorXi known(known_set.size());
    {
        int i = 0;
        for (auto k : known_set) {
            known(i) = k;
            blurDense(i, 0) = MAX(blur.coeff(k, 0), 0.6); //Cutoff due to numerical instability
            i++;
        }
    }

    int nx = width;
    int ny = height;
    Eigen::SparseMatrix<double> G(nx * ny * 2, nx * ny);
    fd_grad(nx, ny, G);

    Eigen::SparseMatrix<double> A = G.transpose() * G;
    Eigen::SparseMatrix<double> Aeq(0, A.rows());

    Eigen::SparseMatrix<double> B(G.rows(), 1);
    Eigen::MatrixXd Beq = Eigen::MatrixXd::Zero(0, 0);

    igl::min_quad_with_fixed(A, B.toDense(), known, blurDense, Aeq, Beq, false, blurMap);
}