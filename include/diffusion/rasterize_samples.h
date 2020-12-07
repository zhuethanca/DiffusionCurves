#pragma once
#include "util/util.h"
#include <vector>
#include <set>
#include "graphics/Point.h"

/**
 * Plots a sequence of samples with data into a vector for sparse matrix creation.
 */
void rasterize_samples(std::vector<Point> &samples, std::vector<int> &segments, std::set<int> &voidSegments,
                       const std::vector<double> &data, size_t width, size_t height,
                       size_t (*index)(size_t, size_t, size_t, size_t), size_t col,
                       std::vector<Tripletd> &matList) ;