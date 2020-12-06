#pragma once

#include <vector>
#include <Eigen/Core>

#include <reconstruction/PixelChain.h>


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
void findBestPath(PolyLine &bestPath, PixelChain chain, MatrixD penalties);