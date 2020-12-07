#include "diffusion/rasterize_samples.h"
#include "diffusion/plot_line.h"

void rasterize_samples(std::vector<Point> &samples, std::vector<int> &segments, std::set<int> &voidSegments,
                       const std::vector<double> &data, size_t width, size_t height,
                       size_t (*index)(size_t, size_t, size_t, size_t), size_t col,
                       std::vector<Tripletd> &matList) {
    int segment = 0;
    for (int i = 0; i + 1 < samples.size(); i++) {
        while (segment + 1 < segments.size() && segments.at(segment) <= i && i < segments.at(segment + 1))
            segment++;
        if (voidSegments.find(segment) != voidSegments.end())
            continue;
        Point &p1 = samples.at(i);
        Point &p2 = samples.at(i + 1);
        double d1 = data.at(i);
        double d2 = data.at(i + 1);
        plot_line(p1, p2, d1, d2, width, height, index, matList, col);
    }
}