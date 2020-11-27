//
// Created by Ethan on 11/26/2020.
//

#include "graphics/ColorCurve.h"
#include <iostream>
#include <graphics/OpenglUtil.h>
#include <sstream>
#include "graphics/Point.h"

ColorCurve::ColorCurve(Bezier &bezier) : pCurve(bezier.pOffset, bezier.segments), nCurve(bezier.nOffset, bezier.segments) {

}

void ColorCurve::update(GLFWwindow *window) {
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
        selectColor();
        epressed = true;
    } else if (estate == GLFW_RELEASE) {
        epressed = false;
    }
}

void ColorCurve::render() {
    if (pControl.empty()) {
        pCurve.render();
    } else {
        std::vector<double> r, g, b;
        pCurve.interp(pControl, extractRed, r);
        pCurve.interp(pControl, extractGreen, g);
        pCurve.interp(pControl, extractBlue, b);
        pCurve.render(r, g, b);
    }
    if (nControl.empty()) {
        nCurve.render();
    } else {
        std::vector<double> r, g, b;
        nCurve.interp(nControl, extractRed, r);
        nCurve.interp(nControl, extractGreen, g);
        nCurve.interp(nControl, extractBlue, b);
        nCurve.render(r, g, b);
    }
}

void ColorCurve::renderHandles() {
    glBegin(GL_QUADS);{
        for (auto &ctrl : pControl) {
            Point p = pCurve.at(ctrl.first);
            if (selectedP && selected == ctrl.first) {
                glColor3f(0.5, 0.25, 0.25);
            } else {
                glColor3f(0, 0, 0);
            }
            drawCenteredRect(p.x, p.y, C_HANDLE_SIZE, C_HANDLE_SIZE);
            glColor4f(ctrl.second.asARGB.r/255.0, ctrl.second.asARGB.g/255.0,
                      ctrl.second.asARGB.b/255.0, ctrl.second.asARGB.a/255.0);
            drawCenteredRect(p.x, p.y, C_HANDLE_SIZE-C_BORDER_THK, C_HANDLE_SIZE-C_BORDER_THK);
        }
        for (auto &ctrl : nControl) {
            Point p = nCurve.at(ctrl.first);
            if (!selectedP && selected == ctrl.first) {
                glColor3f(0.5, 0.25, 0.25);
            } else {
                glColor3f(0, 0, 0);
            }
            drawCenteredRect(p.x, p.y, C_HANDLE_SIZE, C_HANDLE_SIZE);
            glColor4f(ctrl.second.asARGB.r/255.0, ctrl.second.asARGB.g/255.0,
                      ctrl.second.asARGB.b/255.0, ctrl.second.asARGB.a/255.0);
            drawCenteredRect(p.x, p.y, C_HANDLE_SIZE-C_BORDER_THK, C_HANDLE_SIZE-C_BORDER_THK);
        }
    }glEnd();
}

void ColorCurve::onClick(double x, double y) {
    Point newPoint(x, y);

    for (auto &ctrl : pControl) {
        Point ctrlPoint = pCurve.at(ctrl.first);
        if (ctrlPoint.sqdist(newPoint) < C_SELECTION_RADIUS*C_SELECTION_RADIUS) {
            selected = ctrl.first;
            selectedP = true;
            return;
        }
    }

    for (auto &ctrl : nControl) {
        Point ctrlPoint = nCurve.at(ctrl.first);
        if (ctrlPoint.sqdist(newPoint) < C_SELECTION_RADIUS*C_SELECTION_RADIUS) {
            selected = ctrl.first;
            selectedP = false;
            return;
        }
    }

    std::pair<Point, double> pClosest = pCurve.closestSample(newPoint);
    std::pair<Point, double> nClosest = nCurve.closestSample(newPoint);
    double d1 = pClosest.first.sqdist(newPoint);
    double d2 = nClosest.first.sqdist(newPoint);

    if (d1 < d2 && d1 < C_SELECTION_RADIUS*C_SELECTION_RADIUS) {
        std::vector<double> r, g, b;
        pCurve.interp(pControl, extractRed, r);
        pCurve.interp(pControl, extractGreen, g);
        pCurve.interp(pControl, extractBlue, b);

        size_t index = pCurve.atIndex(pClosest.second);

        pControl.emplace(pClosest.second, ARGBInt(1.0, r.at(index), g.at(index), b.at(index)));
        selected = pClosest.second;
        selectedP = true;
    } else if (d2 < C_SELECTION_RADIUS*C_SELECTION_RADIUS){
        std::vector<double> r, g, b;
        nCurve.interp(nControl, extractRed, r);
        nCurve.interp(nControl, extractGreen, g);
        nCurve.interp(nControl, extractBlue, b);

        size_t index = nCurve.atIndex(nClosest.second);

        nControl.emplace(nClosest.second, ARGBInt(1.0, r.at(index), g.at(index), b.at(index)));
        selected = nClosest.second;
        selectedP = false;
    } else {
        selected = INFINITY;
    }
}

void ColorCurve::onRightClick(double x, double y) {
    Point newPoint(x, y);

    for (auto &ctrl : pControl) {
        Point ctrlPoint = pCurve.at(ctrl.first);
        if (ctrlPoint.sqdist(newPoint) < C_SELECTION_RADIUS*C_SELECTION_RADIUS) {
            pControl.erase(ctrl.first);
            return;
        }
    }

    for (auto &ctrl : nControl) {
        Point ctrlPoint = nCurve.at(ctrl.first);
        if (ctrlPoint.sqdist(newPoint) < C_SELECTION_RADIUS*C_SELECTION_RADIUS) {
            nControl.erase(ctrl.first);
            return;
        }
    }
}

void ColorCurve::onDrag(double x, double y) {
    if (selected != INFINITY) {
        Point newPoint(x, y);
        ARGBInt c(0);
        if (selectedP) {
            c = pControl.at(selected);
            pControl.erase(selected);
        } else {
            c = nControl.at(selected);
            nControl.erase(selected);
        }

        std::pair<Point, double> pClosest = pCurve.closestSample(newPoint);
        std::pair<Point, double> nClosest = nCurve.closestSample(newPoint);
        double d1 = pClosest.first.sqdist(newPoint);
        double d2 = nClosest.first.sqdist(newPoint);

        if (d1 < d2) {
            if (pControl.find(pClosest.second) == pControl.end()) {
                pControl.emplace(pClosest.second, c);
                selected = pClosest.second;
                selectedP = true;
            } else {
                if (selectedP)
                    pControl.emplace(selected, c);
                else
                    nControl.emplace(selected, c);
            }
        } else {
            if (nControl.find(nClosest.second) == nControl.end()) {
                nControl.emplace(nClosest.second, c);
                selected = nClosest.second;
                selectedP = false;
            } else {
                if (selectedP)
                    pControl.emplace(selected, c);
                else
                    nControl.emplace(selected, c);
            }
        }
    }
}

void ColorCurve::selectColor() {
    if (selected != INFINITY) {
        printf("Enter Color: ");
        std::string s;
        getline(std::cin, s);
        unsigned int color = std::stoul(s, nullptr, 16);;
        if (selectedP) {
            pControl.at(selected).asInt = color;
        } else {
            nControl.at(selected).asInt = color;
        }
    }
}

ARGBInt::ARGBInt(int i) : asInt(i){
}

ARGBInt::ARGBInt(ubyte a, ubyte r, ubyte g, ubyte b) : asARGB({b, g, r, a}){
}

ARGBInt::ARGBInt(double a, double r, double g, double b) :
    asARGB({static_cast<ubyte>(((int)(b*255))&0xFF), static_cast<ubyte>(((int)(g*255))&0xFF),
            static_cast<ubyte>(((int)(r*255))&0xFF), static_cast<ubyte>(((int)(a*255))&0xFF)}){
}

double extractRed(ARGBInt argb) {
    return argb.asARGB.r/255.0;
}

double extractGreen(ARGBInt argb) {
    return argb.asARGB.g/255.0;
}

double extractBlue(ARGBInt argb) {
    return argb.asARGB.b/255.0;
}