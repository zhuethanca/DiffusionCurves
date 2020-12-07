#pragma once

#include <vector>
#include <opencv2/core/mat.hpp>

#include <reconstruction/PixelChain.h>


/*
 * Returns a series of distinct chains of pixels from the edge images. Each
 * chain represents a whole edge, parametrized into a 1D representation.
 * 
 * Edges of length less than <lengthThreshold> are discarded.
 * 
 * param chains: Output vector of edge pixel chains.
 * param edges: Black-and-white image, where edges are identified by white pixels.
 * param lengthThreshold: Minimum length required for an edge to be returned.
 */
void traceEdgePixels(std::vector<PixelChain>& chains, cv::Mat edges, int lengthThreshold);