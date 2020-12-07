#include "util/argb.h"

ARGBInt::ARGBInt(int i) : asInt(i) {
}

ARGBInt::ARGBInt(ubyte a, ubyte r, ubyte g, ubyte b) : asARGB({b, g, r, a}) {
}

ARGBInt::ARGBInt(double a, double r, double g, double b) :
        asARGB({static_cast<ubyte>(((int) (b * 255)) & 0xFF), static_cast<ubyte>(((int) (g * 255)) & 0xFF),
                static_cast<ubyte>(((int) (r * 255)) & 0xFF), static_cast<ubyte>(((int) (a * 255)) & 0xFF)}) {
}

double extractRed(ARGBInt argb) {
    return argb.asARGB.r / 255.0;
}

double extractGreen(ARGBInt argb) {
    return argb.asARGB.g / 255.0;
}

double extractBlue(ARGBInt argb) {
    return argb.asARGB.b / 255.0;
}