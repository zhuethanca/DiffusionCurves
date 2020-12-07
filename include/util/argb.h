#pragma once
#include "util/util.h"

struct ARGB {
    ubyte b;
    ubyte g;
    ubyte r;
    ubyte a;
};

union ARGBInt {
    uint32_t asInt;
    ARGB asARGB;

    explicit ARGBInt(int i);

    ARGBInt(ubyte a, ubyte r, ubyte g, ubyte b);

    ARGBInt(double a, double r, double g, double b);
};

double extractRed(ARGBInt argb);
double extractGreen(ARGBInt argb);
double extractBlue(ARGBInt argb);