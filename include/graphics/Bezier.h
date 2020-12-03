//
// Created by Ethan on 11/23/2020.
//

#pragma once
#include <GLFW/glfw3.h>
#include "vector"
#include "Point.h"
#include "Curve.h"

#define HANDLE_SIZE 10
#define SELECTION_RADIUS 20
#define SELECT_THK 2

#define DISTANCE_TOLERANCE 0.25

class Bezier {
public:
    Bezier(double offset_dist);

    std::vector<int> segments;
    std::vector<Point> samples;
    std::vector<Point> norms;
    std::vector<Point> nOffset;
    std::vector<Point> pOffset;
    std::vector<Point> handles;
    Point* selected = NULL;

    void update(GLFWwindow* window);
    void renderCurve();
    void renderHandles();

private:
    double offset_dist;

    bool pressed = false;
    bool rpressed = false;

    Curve curve = Curve(samples, segments);

    void onClick(double x, double y);
    void onRightClick(double x, double y);
    void onDrag(double x, double y);

    void updateBezier();
    static void subdivideBezier(double x1, double y1, double x2, double y2,
                                double x3, double y3, double x4, double y4,
                                std::vector<Point>& samples, std::vector<Point>& norms);
};

