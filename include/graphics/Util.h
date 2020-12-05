#pragma once
typedef unsigned char ubyte;
// Custom MIN and MAX macros because sys/param.h in unavailable on Windows.
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

#include <cstddef>

void drawRect(double x, double y, double width, double height);
void drawCenteredRect(double x, double y, double width, double height);
size_t index(size_t x, size_t y, size_t width, size_t height);