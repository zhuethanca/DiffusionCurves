//
// Created by Ethan on 12/3/2020.
//

#include "graphics/GaussianCurve.h"
#include "diffusion/rasterize_samples.h"
#include "diffusion/interpolate_control.h"
#include <map>

GaussianCurve::GaussianCurve(Bezier &bezier) : bezier(bezier),
                                               curve(bezier.samples, bezier.segments, bezier.voidSegments) {
    // re.seed(time(nullptr));
    control.emplace(0, 2);
    control.emplace(1, 0);
    control.emplace(2, 2);
}

void GaussianCurve::update(GLFWwindow *window) {
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
    int estate = glfwGetKey(window, GLFW_KEY_E);
    if (estate == GLFW_PRESS) {
        selectSigma();
        epressed = true;
    } else if (estate == GLFW_RELEASE) {
        epressed = false;
    }
}

double extract(double v);

void GaussianCurve::render() {
    if (control.empty()) {
        curve.render();
    } else {
        double mx = 0;
        for (auto &ctrl : control) {
            mx = MAX(mx, ctrl.second);
        }
        std::vector<double> b;
        interpolate_control(bezier.samples, curve.controlToIndex(control), extract, b);
        if (mx != 0) {
            for (int i = 0; i < b.size(); i ++) {
                b.at(i) /= mx;
            }
        }
        curve.render(b, b, b);
    }
}

void GaussianCurve::renderHandles() {
    double mx = 0;
    for (auto &ctrl : control) {
        mx = MAX(mx, ctrl.second);
    }
    glBegin(GL_QUADS);
    {
        for (auto &ctrl : control) {
            Point p = curve.at(ctrl.first);
            if (selected == ctrl.first) {
                glColor3f(0.5, 0.25, 0.25);
            } else {
                glColor3f(0, 0, 0);
            }
            drawCenteredRect(p.x, p.y, G_HANDLE_SIZE, G_HANDLE_SIZE);
            double b = mx == 0 ? ctrl.second : ctrl.second/mx;
            glColor4f(b, b,
                      b, 1.0);
            drawCenteredRect(p.x, p.y, G_HANDLE_SIZE - G_BORDER_THK, G_HANDLE_SIZE - G_BORDER_THK);
        }
    }
    glEnd();
}

void GaussianCurve::onClick(double x, double y) {
    Point newPoint(x, y);

    for (auto &ctrl : control) {
        Point ctrlPoint = curve.at(ctrl.first);
        if (ctrlPoint.sqdist(newPoint) < G_SELECTION_RADIUS * G_SELECTION_RADIUS) {
            selected = ctrl.first;
            return;
        }
    }

    std::pair<Point, double> closest = curve.closestSample(newPoint);
    double d1 = closest.first.sqdist(newPoint);

    if (d1 < G_SELECTION_RADIUS * G_SELECTION_RADIUS) {
        if (!shift) {
            std::vector<double> b;
            interpolate_control(bezier.samples, curve.controlToIndex(control), extract, b);
            size_t index = curve.atIndex(closest.second);
            control.emplace(closest.second, b.at(index));
        } else {
            control.emplace(closest.second, unif(re));
        }
        selected = closest.second;
    } else {
        selected = INFINITY;
    }
}

void GaussianCurve::onRightClick(double x, double y) {
    Point newPoint(x, y);

    for (auto &ctrl : control) {
        Point ctrlPoint = curve.at(ctrl.first);
        if (ctrlPoint.sqdist(newPoint) < G_SELECTION_RADIUS * G_SELECTION_RADIUS) {
            control.erase(ctrl.first);
            return;
        }
    }
}

void GaussianCurve::onDrag(double x, double y) {
    if (selected != INFINITY) {
        Point newPoint(x, y);
        double c = 0;
        c = control.at(selected);
        control.erase(selected);

        std::pair<Point, double> closest = curve.closestSample(newPoint);

        if (control.find(closest.second) == control.end()) {
            control.emplace(closest.second, c);
            selected = closest.second;
        } else {
            control.emplace(selected, c);
        }
    }
}

void GaussianCurve::selectSigma() {
    if (selected != INFINITY) {
        printf("Enter σ: ");
        std::string s;
        getline(std::cin, s);
        double color = std::stod(s, nullptr);
        control[selected] = color;
    }
}

double gaussdup(const std::vector<double> &dups) {
    return -1;
}