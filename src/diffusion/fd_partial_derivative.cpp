#include "diffusion/fd_partial_derivative.h"

void fd_partial_derivative(const int nx, const int ny,
                           const int dir, Eigen::SparseMatrix<double> &D) {
    int sx = (dir == 0 ? nx - 1 : nx);
    int sy = (dir == 1 ? ny - 1 : ny);
    int m = nx * ny;
    std::vector<Tripletd> tripletList;
    tripletList.reserve(2 * m);
    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            if (i - (nx - sx) >= 0 && j - (ny - sy) >= 0) {
                tripletList.emplace_back(INDEX(i, j, nx),
                                         INDEX(i - (nx - sx), j - (ny - sy)), 1);
                tripletList.emplace_back(INDEX(i, j, nx),
                                         INDEX(i, j), -1);
            }
        }
    }

    D.setFromTriplets(tripletList.begin(), tripletList.end());
}
