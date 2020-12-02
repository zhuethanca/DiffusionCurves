#include "fd_grad.h"

void fd_grad(const int nx, const int ny, Eigen::SparseMatrix<double> & G) {
    std::vector<Tripletd> tripletList;
    tripletList.reserve(2*(nx*ny*2));
    int off = 0;
    for (int dir = 0; dir <= 1; dir ++) {
        int m = nx * ny;
        Eigen::SparseMatrix<double> D(m, nx*ny);
        fd_partial_derivative(nx, ny, dir, D);
        for (int k=0; k<D.outerSize(); ++k)
            for (Eigen::SparseMatrix<double>::InnerIterator it(D,k); it; ++it)
                tripletList.emplace_back(off + it.row(), it.col(), it.value());
        off += m;
    }
    G.setFromTriplets(tripletList.begin(), tripletList.end());
}
