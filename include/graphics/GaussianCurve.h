#pragma once

#include "Bezier.h"
#include "Util.h"
#include <map>
#include <cmath>
#include <random>

#define G_HANDLE_SIZE 10
#define G_SELECTION_RADIUS 10
#define G_SELECT_THK 2
#define G_BORDER_THK 2

class GaussianCurve {
public:
    std::map<double, double> control;

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

    void onClick(double x, double y);
    void onRightClick(double x, double y);
    void onDrag(double x, double y);

    void selectSigma();
};
