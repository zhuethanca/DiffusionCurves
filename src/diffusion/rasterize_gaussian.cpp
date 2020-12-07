#include "diffusion/rasterize_gaussian.h"
#include "util/util.h"
#include "diffusion/interpolate_control.h"
#include "diffusion/rasterize_samples.h"

double extract(double v);

void rasterize_gaussian(std::vector<Point> &samples, std::vector<Point> &norms,
                        std::vector<int> &segments, std::set<int> &voidSegments,
                        const std::map<size_t, double>& control,
                        Eigen::SparseMatrix<double> &data, size_t width, size_t height) {
    data.resize(width * height, 1);
    std::vector<Tripletd> matList;
    {
        std::vector<double> b;
        interpolate_control(samples, control, extract, b);
        rasterize_samples(samples, segments, voidSegments,
                          b, width, height, index, 0, matList);
    }

    data.setFromTriplets(matList.begin(), matList.end(), [] (const double &,const double &b) { return b; });

    data.prune([](const int& row, const int& col, const double & value){
        return 0 <= value;
    });
}

double extract(double v) {
    return MAX(v, 0);
}