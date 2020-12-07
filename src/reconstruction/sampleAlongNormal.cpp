
#include <reconstruction/sampleAlongNormal.h>


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
                             const double distance) {

    const int width = image.cols;
    const int height = image.rows;

    // Traverse the (normalized) normal to a sample point.
    Eigen::Vector2i sample = (point + distance * normal).cast<int>();

    // Check that the sample point is inside the image.
    bool xOutOfBounds = sample.x() < 0 || sample.x() >= width;
    bool yOutOfBounds = sample.y() < 0 || sample.y() >= height;
    if (xOutOfBounds || yOutOfBounds) {
        // Return null to signify that no colour could be sampled here.
        return NULL;
    }

    // Extract an image patch, at most 3 pixels across, around the sample point.
    const int startX = sample.x() == 0 ? 0 : sample.x() - 1;
    const int endX = sample.x() == width - 1 ? width - 1 : sample.x() + 1;
    const int startY = sample.y() == 0 ? 0 : sample.y() - 1;
    const int endY = sample.y() == height - 1 ? height - 1 : sample.y() + 1;

    const int patchWidth = endX - startX + 1;
    const int patchHeight = endY - startY + 1;

    // Compute the mean of each of the LAB channels in the patch.
    double luminanceMean = 0.0;
    double alphaMean = 0.0;
    double betaMean = 0.0;

    for (int dx = 0; dx < patchWidth; dx++) {
        for (int dy = 0; dy < patchHeight; dy++) {
            cv::Vec3b pixel = imageLab.at<cv::Vec3b>(startY + dy, startX + dx);

            luminanceMean += pixel(0);
            alphaMean += pixel(1);
            betaMean += pixel(2);
        }
    }

    luminanceMean /= (patchWidth * patchHeight);
    alphaMean /= (patchWidth * patchHeight);
    betaMean /= (patchWidth * patchHeight);

    // Compute the variance and standard deviation of LAB channels in the patch.
    double luminanceVariance = 0.0;
    double alphaVariance = 0.0;
    double betaVariance = 0.0;

    for (int dx = 0; dx < patchWidth; dx++) {
        for (int dy = 0; dy < patchHeight; dy++) {
            cv::Vec3b pixel = imageLab.at<cv::Vec3b>(startY + dy, startX + dx);

            luminanceVariance += pow(pixel(0) - luminanceMean, 2.0);
            alphaVariance += pow(pixel(1) - alphaMean, 2.0);
            betaVariance += pow(pixel(2) - betaMean, 2.0);
        }
    }

    luminanceVariance /= (patchWidth * patchHeight);
    alphaVariance /= (patchWidth * patchHeight);
    betaVariance /= (patchWidth * patchHeight);

    const double luminanceStdDev = std::sqrt(luminanceVariance);
    const double alphaStdDev = std::sqrt(alphaVariance);
    const double betaStdDev = std::sqrt(betaVariance);

    // Take the sample point's value in each of the LAB channels.
    cv::Vec3b sampleColour = imageLab.at<cv::Vec3b>(sample.y(), sample.x());
    const double luminance = sampleColour(0);
    const double alpha = sampleColour(1);
    const double beta = sampleColour(2);

    // Check that the sample point is within one standard deviation of the
    // mean for all three channels.
    const double luminanceDiff = std::abs(luminance - luminanceMean);
    const double alphaDiff = std::abs(alpha - alphaMean);
    const double betaDiff = std::abs(beta - betaMean);

    if (luminanceDiff > luminanceStdDev || alphaDiff > alphaStdDev || betaDiff > betaStdDev) {
        // Return null to signify that no colour could be sampled here.
        return NULL;
    } else {
        // Return the BGR colour at the sample point.
        return &image.at<cv::Vec3b>(sample.y(), sample.x());
    }
}