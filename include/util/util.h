#pragma once
#include <cstddef>
#include <Eigen/Eigen>

typedef unsigned char ubyte;
// Custom MIN and MAX macros because sys/param.h in unavailable on Windows.
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
typedef Eigen::Triplet<double> Tripletd;

void drawRect(double x, double y, double width, double height);
void drawCenteredRect(double x, double y, double width, double height);
size_t index(size_t x, size_t y, size_t width, size_t height);
size_t indexDx(size_t x, size_t y, size_t width, size_t height);
size_t indexDy(size_t x, size_t y, size_t width, size_t height);