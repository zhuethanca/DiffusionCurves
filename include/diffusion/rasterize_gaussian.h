#pragma once
#include <set>
#include "graphics/Point.h"
#include <vector>
#include <Eigen/Eigen>

/**
 * Given a sequence of offset Bezier curves defined by samples, norms, segments, voidSegments,
 * and blur control points blurControl,
 * Plot the blur curve to data.
 */
void rasterize_gaussian(std::vector<Point> &samples, std::vector<Point> &norms,
                     std::vector<int> &segments, std::set<int> &voidSegments,
                     const std::map<size_t, double>& blurControl,
                     Eigen::SparseMatrix<double> &data, size_t width, size_t height);