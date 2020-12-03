#pragma once

#include "Bezier.h"
#include "Util.h"
#include <map>
#include <cmath>
#include <random>

class GaussianCurve {
public:
    std::map<double, ubyte> control;

    Curve curve;

    explicit GaussianCurve(Bezier& bezier);
    void render();
    void renderHandles();
    void renderToMatrix(Eigen::SparseMatrix<double> &data, size_t width, size_t height);
    void renderNormToMatrix(Eigen::SparseMatrix<double> &data, size_t width, size_t height);
    void update(GLFWwindow *window);

private:
    Bezier& bezier;

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
};
