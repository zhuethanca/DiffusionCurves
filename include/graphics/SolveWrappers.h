#pragma once

#include "graphics/Bezier.h"
#include "graphics/ColorCurve.h"
#include "graphics/GaussianCurve.h"

#include <Eigen/Eigen>

void solve_diffusion(Bezier &bezier, ColorCurve &colorCurve,
                     size_t width, size_t height, Eigen::MatrixXd &rgbImage);

void solve_gaussian(Bezier &bezier, GaussianCurve &gaussianCurve,
                     size_t width, size_t height, Eigen::MatrixXd &blurImage);