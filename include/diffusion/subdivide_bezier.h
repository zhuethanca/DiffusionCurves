#pragma once
#include <vector>
#include "graphics/Point.h"

/**
 * Distance under which a curve is a straight line.
 */
#define DISTANCE_TOLERANCE 0.25

/**
 * Adapted From http://www.antigrain.com/research/adaptive_bezier/index.html
 *
 * That site is not up anymore, here is the WayBackMachine Link
 *
 * https://web.archive.org/web/20120621023536/http://www.antigrain.com/research/adaptive_bezier/index.html
 *
 * **Does not include endpoints**
 *
 * Inputs:
 *      x1, y1,
 *      x2, y2,
 *      x3, y3,
 *      x4, y4, Control Points of Cubic Bezier Curve
 *
 * Outputs:
 *      samples The linear approximation of the bezier curve
 *      norms The norms of the bezier curve at each point
 */
void subdivideBezier(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4,
                     std::vector<Point>& samples, std::vector<Point>& norms);