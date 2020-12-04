#pragma once

#include <vector>
#include "reconstruction/GaussianStack.h"

#include <opencv2/core/mat.hpp>


class EdgeStack {
    private:
        /*
         * A vector of edge images of increasingly Gaussian-blurred images.
         */
        std::vector<cv::Mat> levels;

    public:
        /**
         * Creates a stack of edge images from a Gaussian scale space.
         *
         * Runs Canny edge detection on each image in the Gaussian stack.
         * Edge detection uses low and high thresholds as specified by
         * parameters.
         *
         * param stack: A stack of images from a Gaussian scale space.
         * param lowThreshold: Low edge strength threshold for Canny edges.
         * param highThreshold: High edge strength threshold for Canny edges.
         */
        EdgeStack(GaussianStack stack, double lowThreshold, double highThreshold) {
            int stackHeight = stack.height();

            // Meaningless, nonzero initial value for number of detected edge pixels.
            int nonzeros = 1;

            for (int layer = 0; layer < stackHeight && nonzeros > 0; layer++) {
                cv::Mat image = stack.layer(layer);

                cv::Mat edges;
                cv::Canny(image, edges, lowThreshold, highThreshold);

                this->levels.push_back(edges);
                nonzeros = cv::countNonZero(edges);
            }

            if (nonzeros == 0) {
                // Remove the last image which had zero edge pixels.
                this->levels.pop_back();
            }

            int edgeHeight = this->height();
            if (edgeHeight < stackHeight) {
                // Trim down the Gaussian stack, removing any images that are
                // too blurred to distinguish any detail.
                stack.restrict(edgeHeight);
            }
        }

        /*
         * Returns the number of levels in the edge stack.
         */
        int height() {
            return this->levels.size();
        }

        /*
         * Returns the image of edges at the <layer>'th layer.
         */
        cv::Mat layer(int layer) {
            return this->levels.at(layer);
        }
};