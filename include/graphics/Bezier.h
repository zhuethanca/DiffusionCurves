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

    Bezier(std::vector<Point> polyline, double offsetDist, double tension = 2.0) {
        this->offset_dist = offsetDist;

        const int nPoints = polyline.size();
        Eigen::Vector2f first = polyline.at(0).toVector();
        Eigen::Vector2f second = polyline.at(1).toVector();
        Eigen::Vector2f secondLast = polyline.at(nPoints - 2).toVector();
        Eigen::Vector2f last = polyline.at(nPoints - 1).toVector();

        Eigen::MatrixXf derivatives(2, nPoints);
        derivatives.col(0) = (second - first) / tension;
        derivatives.col(nPoints - 1) = (last - secondLast) / tension;

        for (int i = 1; i < polyline.size() - 1; i++) {
            Eigen::Vector2f next = polyline.at(i + 1).toVector();
            Eigen::Vector2f prev = polyline.at(i - 1).toVector();

            derivatives.col(i) = (next - prev) / tension;
        }

        Eigen::Vector2f firstDerivative = derivatives.col(0);
        Eigen::Vector2f firstControl = first + firstDerivative / 3.0;

        Point firstHandle = polyline.at(0);
        Point firstControlHandle(firstControl(0), firstControl(1));

        this->handles.push_back(firstHandle);
        this->handles.push_back(firstControlHandle);

        for (int i = 1; i < nPoints - 1; i++) {
            Eigen::Vector2f curr = polyline.at(i).toVector();
            Eigen::Vector2f currDerivative = derivatives.col(i);

            Eigen::Vector2f prevControl = curr - currDerivative / 3.0;
            Eigen::Vector2f nextControl = curr + currDerivative / 3.0;

            Point currHandle = polyline.at(i);
            Point prevControlHandle(prevControl(0), prevControl(1));
            Point nextControlHandle(nextControl(0), nextControl(1));

            this->handles.push_back(prevControlHandle);
            this->handles.push_back(currHandle);
            this->handles.push_back(nextControlHandle);
        }

        Eigen::Vector2f lastDerivative = derivatives.col(nPoints - 1);
        Eigen::Vector2f lastControl = last + lastDerivative / 3.0;

        Point lastHandle = polyline.at(nPoints - 1);
        Point lastControlHandle(lastControl(0), lastControl(1));

        this->handles.push_back(lastControlHandle);
        this->handles.push_back(lastHandle);

        this->updateBezier();
    }

    std::vector<int> segments;
    std::set<int> voidSegments;
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
    bool shift = false;

    Curve curve = Curve(samples, segments, voidSegments);

    void onClick(double x, double y);
    void onRightClick(double x, double y);
    void onDrag(double x, double y);

    void updateBezier();
    static void subdivideBezier(double x1, double y1, double x2, double y2,
                                double x3, double y3, double x4, double y4,
                                std::vector<Point>& samples, std::vector<Point>& norms);
};

