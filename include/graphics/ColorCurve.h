#pragma once

#include "Bezier.h"
#include <map>
#include <cmath>

#define C_HANDLE_SIZE 10
#define C_SELECTION_RADIUS 10
#define C_SELECT_THK 2
#define C_BORDER_THK 2

typedef unsigned char ubyte;

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

class ColorCurve {
public:
    explicit ColorCurve(Bezier& bezier);
    void render();
    void renderHandles();
    void update(GLFWwindow *window);

private:
    std::map<double, ARGBInt> pControl;
    std::map<double, ARGBInt> nControl;

    bool epressed = false;
    bool pressed = false;
    bool rpressed = false;
    Curve pCurve;
    Curve nCurve;

    double selected = INFINITY;
    bool selectedP = false;

    void onClick(double x, double y);
    void onRightClick(double x, double y);
    void onDrag(double x, double y);

    void selectColor();
};

double extractRed(ARGBInt argb);

double extractGreen(ARGBInt argb);

double extractBlue(ARGBInt argb);