#pragma once
#include "graphics/Point.h"
#include <vector>
#include <stddef.h>
#include "util/util.h"

/**
 * Bresenham's line algorithm
 * Psudocode Source: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 *
 * Plots a line between p1 and p2, interpolating the data d1, d2 on distance.
 *
 * Inputs:
 *      p1 Starting Point
 *      p2 Ending Point
 *      d1 Data at p1
 *      d2 Data at p2
 *      width Width of the drawable area
 *      height Height of the drawable area
 *      index Indexing function to convert 2d array to 1d column
 *      col Column to write to
 * Outputs:
 *      target Appends Interpolated values on the line
 */
void plot_line(Point &p1, Point &p2, double d1, double d2,
        size_t width, size_t height, size_t (*index)(size_t x, size_t y, size_t width, size_t height),
        std::vector<Tripletd> &target, size_t col);