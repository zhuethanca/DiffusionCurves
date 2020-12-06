#pragma once

#include <graphics/Bezier.h>
#include <graphics/ColorCurve.h>

#include <opencv2/core/mat.hpp>

/*
 * Set the colours alongside a Bezier curve by sampling nearby points in an
 * image. The number of sample points is determined by <sampleDensity> and
 * the approximate length of the curve.
 *
 * param curve: A cubic Bezier curve.
 * param colours: An object signifying colour constraints around the curve.
 * param image: A BGR image.
 * param imageLab: The same image represented in the  Lab colour scheme.
 * param sampleDensity: The number of samples to take per pixel on the curve.
 */
void sampleBezierColours(Bezier &curve,
                         ColorCurve &colours,
                         cv::Mat &image,
                         cv::Mat &imageLab,
                         const double sampleDensity);