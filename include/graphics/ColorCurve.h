#pragma once

#include "Bezier.h"
#include <map>
#include <cmath>
#include <random>
#include "util/util.h"
#include "util/argb.h"

#define C_HANDLE_SIZE 10
#define C_SELECTION_RADIUS 10
#define C_SELECT_THK 2
#define C_BORDER_THK 2

class ColorCurve {
public:
    std::map<double, ARGBInt> pControl;
    std::map<double, ARGBInt> nControl;

    Curve pCurve;
    Curve nCurve;

    explicit ColorCurve(Bezier& bezier);

    ColorCurve(std::vector<Point> &samples, std::vector<int> &segments, std::set<int> &voidSegments,
               std::vector<Point> &nOffset, std::vector<Point> &pOffset, std::vector<Point> &norms);

    void render();
    void renderHandles();
    void update(GLFWwindow *window);

private:
    std::vector<Point>& samples;
    std::vector<int>& segments;
    std::set<int>& voidSegments;
    std::vector<Point> &nOffset;
    std::vector<Point> &pOffset;
    std::vector<Point> &norms;

    std::uniform_real_distribution<double> unif;
    std::default_random_engine re;

    bool shift = false;
    bool epressed = false;
    bool pressed = false;
    bool rpressed = false;

    double selected = INFINITY;
    bool selectedP = false;

    void onClick(double x, double y);
    void onRightClick(double x, double y);
    void onDrag(double x, double y);

    void selectColor();

    void flood_fill(Point start, Eigen::MatrixXi &map, Point corner, std::map<int, std::vector<double>> &out,
                    size_t width, size_t height);
};

double extractRed(ARGBInt argb);

double extractGreen(ARGBInt argb);

double extractBlue(ARGBInt argb);

size_t indexDx(size_t x, size_t y, size_t width, size_t height);
size_t indexDy(size_t x, size_t y, size_t width, size_t height);