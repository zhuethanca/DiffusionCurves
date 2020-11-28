//
// Created by Ethan on 11/26/2020.
//

#include "graphics/ColorCurve.h"
#include <iostream>
#include <graphics/Util.h>
#include <sstream>
#include "graphics/Point.h"

ColorCurve::ColorCurve(Bezier &bezier) : bezier(bezier),
    pCurve(bezier.pOffset, bezier.segments), nCurve(bezier.nOffset, bezier.segments), unif(0, 1) {
    re.seed(time(nullptr));
}

void ColorCurve::update(GLFWwindow *window) {
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
        if (!shift) {
            std::vector<double> r, g, b;
            pCurve.interp(pControl, extractRed, r);
            pCurve.interp(pControl, extractGreen, g);
            pCurve.interp(pControl, extractBlue, b);
            size_t index = pCurve.atIndex(pClosest.second);
            pControl.emplace(pClosest.second, ARGBInt(1.0, r.at(index), g.at(index), b.at(index)));
        } else {
            pControl.emplace(pClosest.second, ARGBInt(1.0, unif(re), unif(re), unif(re)));
        }
        selected = pClosest.second;
        selectedP = true;
    } else if (d2 < C_SELECTION_RADIUS*C_SELECTION_RADIUS){
        if (!shift) {
            std::vector<double> r, g, b;
            nCurve.interp(nControl, extractRed, r);
            nCurve.interp(nControl, extractGreen, g);
            nCurve.interp(nControl, extractBlue, b);

            size_t index = nCurve.atIndex(nClosest.second);

            nControl.emplace(nClosest.second, ARGBInt(1.0, r.at(index), g.at(index), b.at(index)));
        } else {
            nControl.emplace(nClosest.second, ARGBInt(1.0, unif(re), unif(re), unif(re)));
        }
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

double colordup(const std::vector<double> &dups) {
    return dups.at(0);
}

void ColorCurve::renderToMatrix(Eigen::SparseMatrix<double> &data, size_t width, size_t height) {
    data.resize(width * height, 3);
    std::map<int, std::vector<double>> dups;
    std::vector<Tripletd> matList;
    {
        std::vector<double> r, g, b;
        pCurve.interp(pControl, extractRed, r);
        pCurve.interp(pControl, extractGreen, g);
        pCurve.interp(pControl, extractBlue, b);
        pCurve.renderToArray(r, width, height, index, 0, dups, matList, width*height);
        pCurve.renderToArray(g, width, height, index, 1, dups, matList, width*height);
        pCurve.renderToArray(b, width, height, index, 2, dups, matList, width*height);
    }
    {
        std::vector<double> r, g, b;
        nCurve.interp(nControl, extractRed, r);
        nCurve.interp(nControl, extractGreen, g);
        nCurve.interp(nControl, extractBlue, b);
        nCurve.render(r, g, b);
        nCurve.renderToArray(r, width, height, index, 0, dups, matList, width*height);
        nCurve.renderToArray(g, width, height, index, 1, dups, matList, width*height);
        nCurve.renderToArray(b, width, height, index, 2, dups, matList, width*height);
    }
    Curve::finalizeArrayRender(data, colordup, width, height, dups, matList, width*height);
}

size_t indexDx(size_t x, size_t y, size_t width, size_t height) {
    return y*width + x;
}

size_t indexDy(size_t x, size_t y, size_t width, size_t height) {
    return (width*height) + (y*width + x);
}

double normdup(const std::vector<double> &dups) {
    double sum = 0;
    for (double d : dups)
        sum += d;
    return sum / dups.size();
}

void ColorCurve::renderNormToMatrix(Eigen::SparseMatrix<double> &data, size_t width, size_t height) {
    data.resize(width * height*2, 3);
    std::map<int, std::vector<double>> dups;
    std::vector<Tripletd> matList;
    Curve curve(bezier.samples, bezier.segments);
    {
        std::vector<double> pr, pg, pb, nr, ng, nb;
        pCurve.interp(pControl, extractRed, pr);
        pCurve.interp(pControl, extractGreen, pg);
        pCurve.interp(pControl, extractBlue, pb);
        nCurve.interp(nControl, extractRed, nr);
        nCurve.interp(nControl, extractGreen, ng);
        nCurve.interp(nControl, extractBlue, nb);

        std::vector<double> rdx, rdy, gdx, gdy, bdx, bdy;
        for (int i = 0; i < bezier.norms.size(); i ++) {
            Point &norm = bezier.norms.at(i);
            rdx.emplace_back(norm.x * (pr.at(i) - nr.at(i)));
            gdx.emplace_back(norm.x * (pg.at(i) - ng.at(i)));
            bdx.emplace_back(norm.x * (pb.at(i) - nb.at(i)));
            rdy.emplace_back(norm.y * (pr.at(i) - nr.at(i)));
            gdy.emplace_back(norm.y * (pg.at(i) - ng.at(i)));
            bdy.emplace_back(norm.y * (pb.at(i) - nb.at(i)));
        }

        curve.renderToArray(rdx, width, height, indexDx, 0, dups, matList, width*height*2);
        curve.renderToArray(gdx, width, height, indexDx, 1, dups, matList, width*height*2);
        curve.renderToArray(bdx, width, height, indexDx, 2, dups, matList, width*height*2);
        curve.renderToArray(rdy, width, height, indexDy, 0, dups, matList, width*height*2);
        curve.renderToArray(gdy, width, height, indexDy, 1, dups, matList, width*height*2);
        curve.renderToArray(bdy, width, height, indexDy, 2, dups, matList, width*height*2);
    }
    Curve::finalizeArrayRender(data, normdup, width, height, dups, matList, width*height*2);
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