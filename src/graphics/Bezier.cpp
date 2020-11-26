//
// Created by Ethan on 11/23/2020.
//

#include "graphics/Bezier.h"
#include <iostream>
#include <cmath>

void Bezier::update(GLFWwindow *window) {
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
    Point newPoint(x, y);
    for (Point& p : handles) {
        if (p.dist(newPoint) < SELECTION_RADIUS) {
            selected = &p;
            return;
        }
    }
    handles.emplace_back(x, y);
    selected = &*handles.rbegin();
    updateBezier();
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
    if (i != handles.end())
        handles.erase(i);
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
        for (Point &p : handles) {
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
            if (i > 0) {
                Point &p1 = handles.at(i - 1);
                Point &p2 = handles.at(i);
                glVertex2f(p1.x, p1.y);
                glVertex2f(p2.x, p2.y);
            }
            if (i + 1 < handles.size()) {
                Point &p1 = handles.at(i);
                Point &p2 = handles.at(i+1);
                glVertex2f(p1.x, p1.y);
                glVertex2f(p2.x, p2.y);
            }
        }
    }glEnd();
}

void Bezier::renderCurve() {
    if (samples.size() < 4) return;
    glBegin(GL_LINES);{
        glColor3f(0, 0, 0);
        for (int i = 1; i < samples.size(); i ++) {
            Point &p1 = samples.at(i - 1);
            Point &p2 = samples.at(i);
            glVertex2f(p1.x, p1.y);
            glVertex2f(p2.x, p2.y);
        }
    }glEnd();
}

void Bezier::updateBezier() {
    samples.clear();
    norms.clear();
    if (handles.size() < 4)
        return;
    samples.emplace_back(handles.at(0).x, handles.at(0).y);
    norms.emplace_back(handles.at(1).y-handles.at(0).y, handles.at(0).x-handles.at(1).x);
    (*norms.rbegin()).normalize();
    for (int i = 0; i+3 < handles.size(); i += 3) {
        subdivideBezier(handles.at(i+0).x, handles.at(i+0).y,
                        handles.at(i+1).x, handles.at(i+1).y,
                        handles.at(i+2).x, handles.at(i+2).y,
                        handles.at(i+3).x, handles.at(i+3).y,
                        samples, norms);
        samples.emplace_back(handles.at(i+3).x, handles.at(i+3).y);
        norms.emplace_back(handles.at(i+3).y-handles.at(i+2).y, handles.at(i+2).x-handles.at(i+3).x);
        (*norms.rbegin()).normalize();
    }
}

void Bezier::drawRect(double x, double y, double width, double height) {
    glVertex2f(x, y);
    glVertex2f( x+width, y);
    glVertex2f( x+width, y+height);
    glVertex2f(x, y+height);
}

void Bezier::drawCenteredRect(double x, double y, double width, double height) {
    glVertex2f(x-width/2.0, y-height/2.0);
    glVertex2f(x+width/2.0, y-height/2.0);
    glVertex2f(x+width/2.0, y+height/2.0);
    glVertex2f(x-width/2.0, y+height/2.0);
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
        (*norms.rbegin()).normalize();
        return;
    }

    // Continue subdivision
    //----------------------
    subdivideBezier(x1, y1, x12, y12, x123, y123, x1234, y1234, samples, norms);
    subdivideBezier(x1234, y1234, x234, y234, x34, y34, x4, y4, samples, norms);
}
