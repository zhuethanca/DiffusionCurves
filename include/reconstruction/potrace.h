#pragma once

#include <vector>
#include <Eigen/Core>

#include <reconstruction/PixelChain.h>


/*
 * Runs a modified Potrace (polygon trace) algorithm to transform a continuous
 * pixel chain into a polyline with minimal segments while still respecting the
 * shape of the pixel chain.
 *
 * For each pixel, the algorithm attempts to construct a straight line to every
 * other pixel. Each attempt has a penalty based on the standard deviation of
 * pixels between the start and end from the line segment. Lines with too high
 * a penalty are discarded. Short polylines are prioritized over low error
 * among valid paths.
 *
 * param polyline: Return value which will hold the points in the polyline.
 * param chain: Continuous chain of pixels to be approximated by a polyline.
 */
void potrace(std::vector<Point>& polyline, PixelChain chain);