#pragma once

#include <vector>
#include <Eigen/Core>

#include <reconstruction/PixelChain.h>


typedef Eigen::Vector2f Vector2;


void findBestPath(std::vector<int> &bestPath, PixelChain chain, Eigen::MatrixXd penalties) {
    const int nPoints = chain.length();

    Eigen::VectorXd bestPenalties(nPoints);
    bestPenalties.setConstant(-1);
    bestPenalties(nPoints - 1) = 0;

    std::vector<std::vector<int>> bestPaths;

    // Initialize bestPaths with an empty path for each point, except the last.
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

                if (firstPath || shortPath || (equalPath && cheapPath)) {
                    /*
                    std::cout << "Improving on path from vertex " << i << " with length " << bestPaths.at(i).size() << ", penalty " << bestPenalties(i) << " with another of length " << pathCandidate.size() << ", penalty " << penaltyCandidate << std::endl;
                    std::cout << "Start point: (" << chain.at(i).transpose() << ")" << std::endl;
                    std::cout << "New path: ";
                    */
                    // for (int q = 0; q < pathCandidate.size(); q++) {
                    //     std::cout << pathCandidate.at(q) << "(" << chain.at(pathCandidate.at(q)).transpose() << ") ";
                    // }
                    // std::cout << std::endl;

                    bestPenalties(i) = penaltyCandidate;
                    bestPaths.at(i) = pathCandidate;
                }
            }
        }
    }

    bestPath = bestPaths.front();
    std::reverse(bestPath.begin(), bestPath.end());
}


void potrace(std::vector<int> &polyline, PixelChain chain) {
    const int nPoints = chain.length();

    Eigen::MatrixXd penalties(nPoints, nPoints);
    penalties.setConstant(-1.0);

    // Iterate over all triplets i, j, k, such that 0 <= i < j < k <= nPoints.
    for (int i = 0; i < nPoints; i++) {
        for (int k = i + 1; k < nPoints; k++) {
            bool isStraightPath = true;

            Vector2 pointI = chain.get(i).toVector();
            Vector2 pointK = chain.get(k).toVector();

            for (int j = i + 1; j < k; j++) {
                Vector2 pointJ = chain.get(j).toVector();

                Vector2 lineItoK = pointK - pointI;
                Vector2 lineItoJ = pointJ - pointI;

                double coefficient = lineItoJ.dot(lineItoK) / lineItoK.squaredNorm();
                Vector2 dispJFromLine = lineItoJ - coefficient * lineItoK;

                if (dispJFromLine.norm() >= 1.0) {
                    isStraightPath = false;
                    break;
                }
            }

            if (isStraightPath) {
                // Compute penalty for this path segment.
                const double x = pointK.x() - pointI.x();
                const double y = pointK.y() - pointI.y();

                const double xBar = (pointK + pointI).x() / 2.0;
                const double yBar = (pointK + pointI).y() / 2.0;

                double expectedX = 0.0;
                double expectedY = 0.0;
                double expectedXY = 0.0;
                double expectedXSquare = 0.0;
                double expectedYSquare = 0.0;

                for (int p = i; p <= k; p++) {
                    const int xp = chain.get(p).x;
                    const int yp = chain.get(p).y;

                    expectedX += xp;
                    expectedY += yp;
                    expectedXY += xp * yp;
                    expectedXSquare += xp * xp;
                    expectedYSquare += yp * yp;
                }

                expectedX /= (k - i + 1);
                expectedY /= (k - i + 1);
                expectedXY /= (k - i + 1);
                expectedXSquare /= (k - i + 1);
                expectedYSquare /= (k - i + 1);

                /*
                std::cout << "Line from (" << pointI.x() << ", " << pointI.y() << ") to (" << pointK.x() << ", " << pointK.y() << "):" << std::endl;
                std::cout << "Centroid = (" << xBar << ", " << yBar << ")" << std::endl;
                std::cout << "Expected X = " << expectedX << std::endl;
                std::cout << "Expected Y = " << expectedY << std::endl;
                std::cout << "Expected XY = " << expectedXY << std::endl;
                std::cout << "Expected X^2 = " << expectedXSquare << std::endl;
                std::cout << "Expected Y^2 = " << expectedYSquare << std::endl;
                */

                const double a = expectedXSquare - 2 * xBar * expectedX + pow(xBar, 2.0);
                const double b = expectedXY - xBar * expectedX - yBar * expectedY + xBar * yBar;
                const double c = expectedYSquare - 2 * yBar * expectedY + pow(yBar, 2.0);

                /*
                std::cout << "a = " << a << std::endl;
                std::cout << "b = " << b << std::endl;
                std::cout << "c = " << c << std::endl;
                */
                
                const double interior = c * pow(x, 2.0) + 2 * b * x * y + a * pow(y, 2.0);
                const double penalty = std::sqrt(interior);

                penalties(i, k) = penalty;
            }
        }
    }

    // std::vector<int> polyline;
    findBestPath(polyline, chain, penalties);

    /*
    std::cout << "For the chain ";
    for (int i = 0; i < chain.length(); i++) {
        std::cout << "(" << chain.at(i).transpose() << ") ";
    }
    std::cout << ":" << std::endl;

    for (int i = 0; i < polyline.size(); i++) {
        std::cout << "Polyline stops at (" << chain.at(polyline.at(i)).x() << ", " << chain.at(polyline.at(i)).y() << ")" << std::endl;
    }
    */
}