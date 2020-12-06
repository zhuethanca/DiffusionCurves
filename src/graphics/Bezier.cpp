//
// Created by Ethan on 11/23/2020.
//

#include "graphics/Bezier.h"
#include "../../opengl.h"
#include <iostream>
#include <cmath>
#include <graphics/Util.h>

Bezier::Bezier(double offsetDist) : offset_dist(offsetDist) {
    /*
    Handles for project demo.

    handles.emplace_back(227, 533);
    handles.emplace_back(440, 531);
    handles.emplace_back(296, 238);
    handles.emplace_back(521, 237);
    handles.emplace_back(677, 240);
    handles.emplace_back(639, 528);
    handles.emplace_back(798, 531);
    updateBezier();
    */
}

Bezier::Bezier(std::vector<Point> polyline, double offsetDist, double tension) {
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

    // Correct the normal around the first handle.
    Eigen::Vector2f lastDerivative = derivatives.col(nPoints - 1);
    Eigen::Vector2f lastControl = last - lastDerivative / 3.0;

    Point lastHandle = polyline.at(nPoints - 1);
    Point lastControlHandle(lastControl(0), lastControl(1));

    this->handles.push_back(lastControlHandle);
    this->handles.push_back(lastHandle);

    this->updateBezier();
}

void Bezier::update(GLFWwindow *window) {
    int sftstate = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    if (sftstate == GLFW_PRESS) {
        shift = true;
    } else if (sftstate == GLFW_RELEASE) {
        shift = false;
    }
    int lstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (lstate == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        if (!pressed) {
            onClick(x, y);
            pressed = true;
        } else {
            onDrag(x, y);
        }
    } else if (lstate == GLFW_RELEASE) {
        pressed = false;
    }
    int rstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (rstate == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        if (!rpressed) {
            onRightClick(x, y);
            rpressed = true;
        }
    } else if (rstate == GLFW_RELEASE) {
        rpressed = false;
    }
}

void Bezier::onClick(double x, double y) {
    if (!shift) {
        Point newPoint(x, y);
        for (Point &p : handles) {
            if (p.dist(newPoint) < SELECTION_RADIUS) {
                selected = &p;
                return;
            }
        }
        handles.emplace_back(x, y);
        selected = &*handles.rbegin();
        updateBezier();
    } else {
        handles.emplace_back(INFINITY, INFINITY);
        selected = NULL;
        updateBezier();
    }
}

void Bezier::onRightClick(double x, double y) {
    Point newPoint(x, y);
    auto i = handles.begin();
    for (; i != handles.end(); i ++) {
        Point& p = *i;
        if (p.dist(newPoint) < SELECTION_RADIUS) {
            break;
        }
    }
    if (i != handles.end() && i+1 == handles.end()) {
        handles.erase(i);
        selected = NULL;
    }
    updateBezier();
}

void Bezier::onDrag(double x, double y) {
    if (selected != NULL) {
        selected->x = x;
        selected->y = y;
    }
    updateBezier();
}

void Bezier::renderHandles() {
    glBegin(GL_QUADS);{
        glColor3f(0, 0, 0);
        for (Point &p : handles) if (!std::isinf(p.x)){
            drawCenteredRect(p.x, p.y, HANDLE_SIZE, HANDLE_SIZE);
        }

        if (selected != nullptr) {
            glColor3f(1, 1, 1);
            drawCenteredRect(selected->x, selected->y, HANDLE_SIZE - SELECT_THK, HANDLE_SIZE - SELECT_THK);
        }
    }glEnd();
    glBegin(GL_LINES);{
        glColor3f(0, 0, 1);
        for (int i = 0; i < handles.size(); i += 3) {
            str:
            if (i > 0 && !std::isinf(handles.at(i-1).x)) {
                Point &p1 = handles.at(i - 1);
                Point &p2 = handles.at(i);
                glVertex2f(p1.x, p1.y);
                glVertex2f(p2.x, p2.y);
            }
            if (i + 1 < handles.size() && !std::isinf(handles.at(i+1).x)) {
                Point &p1 = handles.at(i);
                Point &p2 = handles.at(i+1);
                glVertex2f(p1.x, p1.y);
                glVertex2f(p2.x, p2.y);
            }
            for (int j = i; j <= i + 2; j ++) if (j < handles.size()){
                if (std::isinf(handles.at(j).x) || std::isinf(handles.at(j).y)) {
                    i = j + 1;
                    if (i < handles.size())
                        goto str;
                }
            }
        }
    }glEnd();
}

void Bezier::renderCurve() {
    this->curve.render();
}

void Bezier::updateBezier() {
    samples.clear();
    norms.clear();
    segments.clear();
    if (handles.size() < 4)
        return;
    segments.push_back(samples.size());

    samples.emplace_back(handles.at(0).x, handles.at(0).y);
    norms.emplace_back(0, 0);

    for (int i = 0; i+3 < handles.size(); i += 3) {
        bool ctn = false;
        int li = i;
        for (int j = li; j <= li + 3; j ++) {
            if (std::isinf(handles.at(j).x) || std::isinf(handles.at(j).y)) {
                i = j+1;
                if (!ctn) {
                    segments.push_back(samples.size());
                    voidSegments.emplace(segments.size()-1);
                    samples.emplace_back(handles.at(i).x, handles.at(i).y);
                    norms.emplace_back(0, 0);
                    ctn = true;
                }
            }
        }
        if (i + 3 >= handles.size()) {
            break;
        }

        subdivideBezier(handles.at(i+0).x, handles.at(i+0).y,
                        handles.at(i+1).x, handles.at(i+1).y,
                        handles.at(i+2).x, handles.at(i+2).y,
                        handles.at(i+3).x, handles.at(i+3).y,
                        samples, norms);
        segments.push_back(samples.size());

        samples.emplace_back(handles.at(i+3).x, handles.at(i+3).y);
        norms.emplace_back(handles.at(i+3).y-handles.at(i+2).y, handles.at(i+2).x-handles.at(i+3).x);

        Point& norm = (*norms.rbegin());
        norm.normalize();
    }

    Point& firstNorm = norms.front();

    Point newFirstNorm(handles.at(1).y - handles.at(0).y, handles.at(0).x - handles.at(1).x);
    newFirstNorm.normalize();

    firstNorm.x += newFirstNorm.x;
    firstNorm.y += newFirstNorm.y;
    firstNorm.normalize();

    curve.offset(norms, offset_dist, pOffset);
    curve.offset(norms, -offset_dist, nOffset);
}

/**
 * Adapted From http://www.antigrain.com/research/adaptive_bezier/index.html
 *
 * That site is not up anymore, here is the WayBackMachine Link
 *
 * https://web.archive.org/web/20120621023536/http://www.antigrain.com/research/adaptive_bezier/index.html
 */
void Bezier::subdivideBezier(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4,
                             std::vector<Point>& samples, std::vector<Point>& norms) {
    // Calculate all the mid-points of the line segments
    //----------------------
    double x12   = (x1 + x2) / 2;
    double y12   = (y1 + y2) / 2;
    double x23   = (x2 + x3) / 2;
    double y23   = (y2 + y3) / 2;
    double x34   = (x3 + x4) / 2;
    double y34   = (y3 + y4) / 2;
    double x123  = (x12 + x23) / 2;
    double y123  = (y12 + y23) / 2;
    double x234  = (x23 + x34) / 2;
    double y234  = (y23 + y34) / 2;
    double x1234 = (x123 + x234) / 2;
    double y1234 = (y123 + y234) / 2;

    // Try to approximate the full cubic curve by a single straight line
    //------------------
    double dx = x4-x1;
    double dy = y4-y1;

    double d2 = fabs(((x2 - x4) * dy - (y2 - y4) * dx));
    double d3 = fabs(((x3 - x4) * dy - (y3 - y4) * dx));

    if((d2 + d3)*(d2 + d3) < DISTANCE_TOLERANCE * (dx*dx + dy*dy))
    {
        samples.emplace_back(x1234, y1234);
        norms.emplace_back(y234-y1234, x1234-x234);
        Point& norm = (*norms.rbegin());
        if (abs(norm.x) < 1E-4 && abs(norm.y) < 1E-4) {
            norm.x = (y2-y1);
            norm.y = (x1-x2);
        }
        norm.normalize();
        return;
    }

    // Continue subdivision
    //----------------------
    subdivideBezier(x1, y1, x12, y12, x123, y123, x1234, y1234, samples, norms);
    subdivideBezier(x1234, y1234, x234, y234, x34, y34, x4, y4, samples, norms);
}
