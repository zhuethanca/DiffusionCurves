#pragma once
#include <Eigen/Eigen>
#include <set>
#include "graphics/Point.h"
#include "util/argb.h"

/**
 * Given a sequence of offset Bezier curves defined by segments, voidSegments,
 * pOffset, nOffset, and color control points pControl, nControl,
 * Plot the color curve to data.
 */
void rasterize_color(std::vector<int> &segments, std::set<int> &voidSegments,
                    std::vector<Point> &pOffset, std::vector<Point> &nOffset,
                    const std::map<size_t, ARGBInt>& pControl, const std::map<size_t, ARGBInt>& nControl,
                    Eigen::SparseMatrix<double> &data, size_t width, size_t height);