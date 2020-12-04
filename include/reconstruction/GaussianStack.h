#pragma once

#include <vector>
#include <opencv2/core/mat.hpp>


class GaussianStack {
    private:
        /*
        * A vector of increasingly Gaussian-blurred images.
        */
        std::vector<cv::Mat> levels;

    public:
        /**
         * Construct a Gaussian scale space representing the image passed in.
         *
         * Repeatedly applies a Gaussian filter with radius sigma starting from 0
         * and increasing by <sigmaStep> (default 0.4) each level. The stack stops
         * when the image's standard deviation drops below <stdDevCutoff> (default
         * 40) or, if a max height is specified, at that height.
         *
         * param image: An OpenCV matrix containing an RGB image.
         * param stdDevCutoff: The minimum standard deviation of a blurred image
         *                     that will be used in the stack.
         * param maxHeight: The maximum height of the stack.
         * param sigmaStep: The increase in Gaussian filter widths between each level.
         */
        GaussianStack(cv::Mat image, double stdDevCutoff = 40.0, int maxHeight = -1, double sigmaStep = 0.4);

        /*
        * Returns the number of levels in the scale space.
        */
        int height();

        /*
        * Restricts the height of the stack to <layers> layers, removing any
        * images above that level.
        */
        void restrict(int layers);

        /*
        * Returns the blurred RGB image at the <layer>'th layer.
        */
        cv::Mat layer(int layer);
};