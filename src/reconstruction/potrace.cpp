
#include <reconstruction/findBestPath.h>
#include <reconstruction/potrace.h>


typedef Eigen::Vector2f Vector2;


/*
 * Runs a modified Potrace (polygon trace) algorithm to transform a continuous
 * pixel chain into a polyline with minimal segments while still respecting the
 * shape of the pixel chain.
 * 
 * For each pixel, the algorithm attempts to construct a straight line to every
 * other pixel. Each attempt has a penalty based on the standard deviation of
 * pixels between the start and end from the line segment. Lines with too high
 * a penalty are discarded. Short polylines are prioritized over low error
 * among valid paths.
 * 
 * param polyline: Return value which will hold the points in the polyline.
 * param chain: Continuous chain of pixels to be approximated by a polyline.
 */
void potrace(std::vector<Point> &polyline, PixelChain chain) {
    const int nPoints = chain.length();

    // Each entry i, j represents the penalty of a straight line
    // from pixel i directly to pixel j.
    Eigen::MatrixXd penalties(nPoints, nPoints);
    // Set a recognizable dummy value to signify no penalty is set.
    penalties.setConstant(-1.0);

    // First, find a penalty between each pair of pixels, or leave the dummy
    // value if two pixels cannot be connected by a straight line.

    // Iterate over all triplets i, j, k, such that 0 <= i < j < k <= nPoints.
    for (int i = 0; i < nPoints; i++) {
        for (int k = i + 1; k < nPoints; k++) {
            bool isStraightPath = true;

            Vector2 pointI = chain.get(i).toVector();
            Vector2 pointK = chain.get(k).toVector();

            // Check that all points between I and K are close enough to the
            // straight line connecting them.
            for (int j = i + 1; j < k; j++) {
                Vector2 pointJ = chain.get(j).toVector();

                Vector2 lineItoK = pointK - pointI;
                Vector2 lineItoJ = pointJ - pointI;

                double coefficient = lineItoJ.dot(lineItoK) / lineItoK.squaredNorm();
                Vector2 dispJFromLine = lineItoJ - coefficient * lineItoK;

                // Discard the line if any point J is further than one unit
                // off the line.
                if (dispJFromLine.norm() >= 1.0) {
                    isStraightPath = false;
                    break;
                }
            }

            if (isStraightPath) {
                // Now that this line is known to be valid, compute the penalty
                // for this path segment.

                // Use an approximation of penalty from the Potrace paper.
                const double x = pointK.x() - pointI.x();
                const double y = pointK.y() - pointI.y();

                const double xBar = (pointK + pointI).x() / 2.0;
                const double yBar = (pointK + pointI).y() / 2.0;

                // Compute expected values of all the terms below.
                double expectedX = 0.0;
                double expectedY = 0.0;
                double expectedXY = 0.0;
                double expectedXSquare = 0.0;
                double expectedYSquare = 0.0;

                for (int j = i; j <= k; j++) {
                    const int xAtj = chain.get(j).x;
                    const int yAtj = chain.get(j).y;

                    expectedX += xAtj;
                    expectedY += yAtj;
                    expectedXY += xAtj * yAtj;
                    expectedXSquare += xAtj * xAtj;
                    expectedYSquare += yAtj * yAtj;
                }

                expectedX /= (k - i + 1);
                expectedY /= (k - i + 1);
                expectedXY /= (k - i + 1);
                expectedXSquare /= (k - i + 1);
                expectedYSquare /= (k - i + 1);

                // Evaluate the penalty approximation from the Potrace paper.
                const double a = expectedXSquare - 2 * xBar * expectedX + pow(xBar, 2.0);
                const double b = expectedXY - xBar * expectedX - yBar * expectedY + xBar * yBar;
                const double c = expectedYSquare - 2 * yBar * expectedY + pow(yBar, 2.0);

                const double interior = c * pow(x, 2.0) + 2 * b * x * y + a * pow(y, 2.0);
                const double penalty = std::sqrt(interior);

                penalties(i, k) = penalty;
            }
        }
    }

    // Search for the shortest and least-penalty path using the penalties
    // matrix. Invalid paths are now identified by dummy values left over
    // in the matrix.
    findBestPath(polyline, chain, penalties);
}