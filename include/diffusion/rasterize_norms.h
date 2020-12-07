#pragma once
#include <set>
#include "graphics/Point.h"
#include "util/argb.h"

void rasterize_norms(std::vector<Point> &samples, std::vector<Point> &norms,
                     std::vector<int> &segments, std::set<int> &voidSegments,
                     std::vector<Point> &pOffset, std::vector<Point> &nOffset,
                     const std::map<size_t, ARGBInt>& pControl, const std::map<size_t, ARGBInt>& nControl,
                     Eigen::SparseMatrix<double> &data, size_t width, size_t height);