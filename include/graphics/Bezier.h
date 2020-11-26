//
// Created by Ethan on 11/23/2020.
//

#pragma once
#include <GLFW/glfw3.h>
#include "vector"
#include "graphics/Point.h"

#define HANDLE_SIZE 10
#define SELECTION_RADIUS 20
#define SELECT_THK 2

#define DISTANCE_TOLERANCE 0.25

class Bezier {
public:
    void update(GLFWwindow* window);
    void renderCurve();
    void renderHandles();

private:
    void onClick(double x, double y);
    void onRightClick(double x, double y);
    void onDrag(double x, double y);

    void updateBezier();
    static void subdivideBezier(double x1, double y1, double x2, double y2,
                                double x3, double y3, double x4, double y4,
                                std::vector<Point>& samples, std::vector<Point>& norms);

    static void drawRect(double x, double y, double width, double height);
    static void drawCenteredRect(double x, double y, double width, double height);
    bool pressed = false;
    bool rpressed = false;
    std::vector<Point> handles;

    std::vector<Point> samples;
    std::vector<Point> norms;
    Point* selected = NULL;
};

