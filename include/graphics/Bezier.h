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

class Bezier {
public:
    Bezier(double offset_dist);

    void load_polyline(std::vector<std::vector<Point>> polylines, double offsetDist, double tension = 2.0);

    std::vector<int> segments;
    std::set<int> voidSegments;
    std::vector<size_t> curveSegment;

    std::vector<Point> samples;
    std::vector<Point> norms;
    std::vector<Point> nOffset;
    std::vector<Point> pOffset;
    std::vector<Point> handles;
    int selectedIndex;

    void update(GLFWwindow* window);
    void renderCurve();
    void renderHandles();
    size_t getCurveSegment(size_t curveIndex);

private:
    double offset_dist;

    bool pressed = false;
    bool rpressed = false;
    bool shift = false;

    Curve curve = Curve(samples, segments, voidSegments);

    void onClick(double x, double y);
    void onRightClick(double x, double y);
    void onDrag(double x, double y);

    void updateBezier();
};

