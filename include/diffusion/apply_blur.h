#pragma once
#include <Eigen/Eigen>

void apply_blur(Eigen::MatrixXd &rgbImage, Eigen::MatrixXd &blurMap,
                    size_t width, size_t height, Eigen::MatrixXd &blurredImage);