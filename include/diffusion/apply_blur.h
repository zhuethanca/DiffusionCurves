#pragma once
#include <Eigen/Eigen>

/**
 * Given the sharp rgb image, and the blur kernel size map, blurMap,
 * Compute blurredImage by applying gaussian blur.
 */
void apply_blur(Eigen::MatrixXd &rgbImage, Eigen::MatrixXd &blurMap,
                    size_t width, size_t height, Eigen::MatrixXd &blurredImage);