#pragma once

#include <Eigen/Core>
#include <opencv2/core/mat.hpp>


typedef Eigen::Vector2f Vector2;


/*
 * Locates a sample point by traversing <distance> pixels from starting point
 * <point> along the unit normal given by <normal>, and returns the BGR colour
 * at that pixel in an image.
 *
 * If the pixel appears to be an outlier, or if it is outside the image, then
 * NULL is returned instead.
 *
 * param image: A BGR image.
 * param imageLab: The same image represented in the  Lab colour scheme.
 * param point: A pixel location in the images.
 * param normal: The direction in which to look for colour samples.
 * param distance: The distance to travel in the normal direction for a sample.
 */
cv::Vec3b* sampleAlongNormal(cv::Mat &image,
                             cv::Mat &imageLab,
                             Vector2 point,
                             Vector2 normal,
                             const double distance = 3.0);