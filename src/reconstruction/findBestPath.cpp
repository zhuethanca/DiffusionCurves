
#include <reconstruction/findBestPath.h>


typedef Eigen::MatrixXd MatrixD;
typedef std::vector<Point> PolyLine;


/*
 * Finds a sequence of pixels, forming a polyline, which approximates
 * the pixel chain <chain> with the minimum number of segments and with
 * the minimum penalty given the matrix <penalties>.
 * 
 * param bestPath: Return value, a sequence of points approximating the chain.
 * param chain: Continuous chain of pixels to be approximated by a polyline.
 * param penalties: Matrix in which the entry i, j represents the penalty of
 *                  a straight line between pixels i and j, or -1 if they are
 *                  not connectable by a straight line.
 */
void findBestPath(PolyLine& bestPath, PixelChain chain, MatrixD penalties) {
    const int nPoints = chain.length();

    // Store a vector which identifies best paths and their penalties from
    // pixel i to the endpoint.
    Eigen::VectorXd bestPenalties(nPoints);
    std::vector<std::vector<int>> bestPaths;

    // Make sure to set initial values for both penalties and paths, including
    // a sensible initial value for the endpoint itself.
    bestPenalties.setConstant(-1);
    bestPenalties(nPoints - 1) = 0;

    for (int i = 0; i < nPoints; i++) {
        std::vector<int> nullPath;
        bestPaths.push_back(nullPath);
    }

    bestPaths.at(nPoints - 1).push_back(nPoints - 1);

    // Work backwards, finding best paths from the end back to the beginning
    // using a dynamic programming approach.
    for (int i = nPoints - 2; i >= 0; i--) {
        for (int j = i + 1; j < nPoints; j++) {
            if (penalties(i, j) != -1) {
                const double penaltyCandidate = penalties(i, j) + bestPenalties(j);
                std::vector<int> pathCandidate = bestPaths.at(j);
                pathCandidate.push_back(i);

                bool firstPath = bestPaths.at(i).size() == 0;
                bool shortPath = pathCandidate.size() < bestPaths.at(i).size();
                bool equalPath = pathCandidate.size() == bestPaths.at(i).size();
                bool cheapPath = penaltyCandidate < bestPenalties(i);

                // Check if this is a new best path for any of the above reasons.
                if (firstPath || shortPath || (equalPath && cheapPath)) {
                    bestPenalties(i) = penaltyCandidate;
                    bestPaths.at(i) = pathCandidate;
                }
            }
        }
    }

    std::vector<int> bestPathIndices = bestPaths.front();
    std::reverse(bestPathIndices.begin(), bestPathIndices.end());

    // Convert the path indices into a polyline.
    bestPath.clear();
    for (int i = 0; i < bestPathIndices.size(); i++) {
        Point point = chain.get(bestPathIndices.at(i));
        bestPath.push_back(point);
    }
}