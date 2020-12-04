
#include "reconstruction/GaussianStack.h"

#include <opencv2/core/mat.hpp>
#include "opencv2/core/hal/interface.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


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
GaussianStack::GaussianStack(cv::Mat image, double stdDevCutoff, int maxHeight, double sigmaStep) {
	this->levels.clear();

    double sigma;
	double stdDev;

    int remainingHeight = maxHeight;

    do {
        // Define parameters for the next level of Gaussian filter.
        const int radius = std::ceil(2 * sigma);
        const int width = 2 * radius + 1;

        cv::Mat blurred;
        cv::GaussianBlur(image, blurred, cv::Size(width, width), sigma, sigma);

        // Compute the image's standard deviation, averaged over three channels.
        cv::Scalar mean, stdDevChannels;
        cv::meanStdDev(blurred, mean, stdDevChannels);

        stdDev = (stdDevChannels[0] + stdDevChannels[1] + stdDevChannels[2]) / 3.0;
        sigma += 0.4;

        this->levels.push_back(blurred);

        remainingHeight -= 1;
    } while (stdDev >= stdDevCutoff && remainingHeight != 0);
}


/*
 * Returns the number of levels in the scale space.
 */
int GaussianStack::height() {
    return this->levels.size();
}


/*
 * Restricts the height of the stack to <layers> layers, removing any
 * images above that level.
 */
void GaussianStack::restrict(int layers) {
    this->levels.resize(layers);
}

/*
 * Returns the blurred RGB image at the <layer>'th layer.
 */
cv::Mat GaussianStack::layer(int layer) {
    return this->levels.at(layer);
}