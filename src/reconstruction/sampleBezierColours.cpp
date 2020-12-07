
#include <reconstruction/sampleAlongNormal.h>
#include <reconstruction/sampleBezierColours.h>


static std::default_random_engine rng;


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
                         ColorCurve& colours,
                         cv::Mat &image,
                         cv::Mat &imageLab,
                         const double sampleDensity) {
    const int width = image.cols;
    const int height = image.rows;

    colours.pControl.clear();
    colours.nControl.clear();

    int handlesIndex = 0;
    for (int curveIndex = 0; handlesIndex < curve.handles.size(); curveIndex++) {
        std::vector<Point> handles;
        for (; handlesIndex < curve.handles.size(); handlesIndex ++) {
            if (std::isinf(curve.handles.at(handlesIndex).x) || std::isinf(curve.handles.at(handlesIndex).y))
                continue;
            else
                break;
        }
        for (; handlesIndex < curve.handles.size(); handlesIndex ++) {
            if (std::isinf(curve.handles.at(handlesIndex).x) || std::isinf(curve.handles.at(handlesIndex).y))
                break;
            else
                handles.push_back(curve.handles.at(handlesIndex));
        }
        const int nCurves = handles.size() / 3;  // Integer division to round down.
        if (nCurves <= 0)
            continue;

        // Precompute the lengths and cumulative lengths up to each handle in the curve.
        Eigen::VectorXi curveLengths(nCurves);
        Eigen::VectorXi cumulativeLengths(nCurves);
        int totalCurveLength = 0;

        for (int c = 0; c < nCurves; c++) {
            Eigen::Vector2f handle1 = handles.at(3 * c).toVector();
            Eigen::Vector2f handle2 = handles.at(3 * c + 3).toVector();

            // Approximate the curve by a straight line, since it was built from
            // a polyline.
            const double length = (handle2 - handle1).norm();
            totalCurveLength += length;

            curveLengths(c) = length;
            cumulativeLengths(c) = totalCurveLength;
        }

        // Take a number of samples proportinal to the total length of the curve.
        const int nSamples = std::ceil(totalCurveLength * sampleDensity);
        std::uniform_real_distribution<double> distribution(0.0, totalCurveLength);

        // Record pixels that have already been checked for colour.
        std::set<Point> visited;

        std::vector<cv::Vec3b> pColours;
        std::vector<double> pPositions;

        std::vector<cv::Vec3b> nColours;
        std::vector<double> nPositions;

        // The positive direction is a 90 degrees to the right.
        Eigen::Matrix2f rotation;
        rotation << 0, 1,
                -1, 0;

        // Continue to sample more random points until enough have been found or
        // all have been tried.
        while ((pColours.size() < nSamples || nColours.size() < nSamples)
               && visited.size() < totalCurveLength) {

            // Generate a random position along the curve.
            double samplePosition = distribution(rng);

            // Identify exactly where on which curve the position belongs to.
            int curveId;
            int prevCurveLength = 0;
            for (curveId = 0; curveId < nCurves; curveId++) {
                if (samplePosition <= cumulativeLengths(curveId)) {
                    break;
                }
                prevCurveLength = cumulativeLengths(curveId);
            }

            double curveLength = curveLengths(curveId);
            double curveProgress = samplePosition - prevCurveLength;

            double curvePercent = curveProgress / curveLength;

            Eigen::Vector2f handle1 = handles.at(3 * curveId).toVector();
            Eigen::Vector2f control1 = handles.at(3 * curveId + 1).toVector();
            Eigen::Vector2f control2 = handles.at(3 * curveId + 2).toVector();
            Eigen::Vector2f handle2 = handles.at(3 * curveId + 3).toVector();

            // Translate the parametrized coordinate to a pixel on the curve.
            Eigen::Vector2f term1 = pow(1 - curvePercent, 3.0) * handle1;
            Eigen::Vector2f term2 = 3 * pow(1 - curvePercent, 2.0) * curvePercent * control1;
            Eigen::Vector2f term3 = 3 * (1 - curvePercent) * pow(curvePercent, 2.0) * control2;
            Eigen::Vector2f term4 = pow(curvePercent, 3.0) * handle2;

            Eigen::Vector2f point = term1 + term2 + term3 + term4;

            // Translate the parametrized coordinate to the tangent at that point.
            term1 = 3 * pow(1 - curvePercent, 2.0) * (control1 - handle1);
            term2 = 6 * (1 - curvePercent) * curvePercent * (control2 - control1);
            term3 = 3 * pow(curvePercent, 2.0) * (handle2 - control2);

            Eigen::Vector2f tangent = (term1 + term2 + term3).normalized();

            // Rotate the tangent to get "positive" and "negative" unit normals.
            Eigen::Vector2f pNormal = rotation * tangent;
            Eigen::Vector2f nNormal = -pNormal;

            if (pColours.size() < nSamples) {
                // Look for a new "positive" colour sample at this point.
                cv::Vec3b *pColour = sampleAlongNormal(image, imageLab, point, pNormal);

                if (pColour != NULL) {
                    // Colour sample is valid.
                    pPositions.push_back(curveId + curvePercent);
                    pColours.push_back(*pColour);
                }
            }

            if (nColours.size() < nSamples) {
                // Look for a new "negative" colour sample at this point.
                cv::Vec3b *nColour = sampleAlongNormal(image, imageLab, point, nNormal);

                if (nColour != NULL) {
                    // Colour sample is valid.
                    nPositions.push_back(curveId + curvePercent);
                    nColours.push_back(*nColour);
                }
            }
        }

        // Publish colour points on the "positive" side of the curve.
        for (int i = 0; i < pColours.size(); i++) {
            const double position = pPositions.at(i);
            cv::Vec3b colourBGR = pColours.at(i);

            ARGBInt colourARGB(1, colourBGR(2), colourBGR(1), colourBGR(0));
            colours.pControl.emplace(curve.getCurveSegment(curveIndex) + position, colourARGB);
        }

        // Publish colour points on the "negative" side of the curve.
        for (int i = 0; i < nColours.size(); i++) {
            const double position = nPositions.at(i);
            cv::Vec3b colourBGR = nColours.at(i);

            ARGBInt colourARGB(1, colourBGR(2), colourBGR(1), colourBGR(0));
            colours.nControl.emplace(curve.getCurveSegment(curveIndex) + position, colourARGB);
        }
    }
}