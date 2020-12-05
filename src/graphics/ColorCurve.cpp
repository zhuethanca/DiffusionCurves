//
// Created by Ethan on 11/26/2020.
//

#include "graphics/ColorCurve.h"
#include <iostream>
#include <graphics/Util.h>
#include <queue>


// Custom MIN and MAX macros because sys/param.h in unavailable on Windows.
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))


ColorCurve::ColorCurve(Bezier &bezier) : samples(bezier.samples), segments(bezier.segments), voidSegments(bezier.voidSegments),
                                         pOffset(bezier.pOffset), nOffset(bezier.nOffset), norms(bezier.norms),
                                         pCurve(bezier.pOffset, bezier.segments, bezier.voidSegments),
                                         nCurve(bezier.nOffset, bezier.segments, bezier.voidSegments), unif(0, 1) {
    re.seed(time(nullptr));
    pControl.emplace(0, 4280620797);
    pControl.emplace(1, 0x00FF00);
    nControl.emplace(0, 4292560705);
    nControl.emplace(1, 4283617022);
}

ColorCurve::ColorCurve(std::vector<Point> &samples, std::vector<int> &segments, std::set<int> &voidSegments,
                       std::vector<Point> &nOffset, std::vector<Point> &pOffset, std::vector<Point> &norms) :
                       samples(samples), segments(segments), voidSegments(voidSegments), nOffset(nOffset),
                       pOffset(pOffset), norms(norms), pCurve(pOffset, segments, voidSegments),
                       nCurve(nOffset, segments, voidSegments), unif(0, 1) {
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
    glBegin(GL_QUADS);
    {
        for (auto &ctrl : pControl) {
            Point p = pCurve.at(ctrl.first);
            if (selectedP && selected == ctrl.first) {
                glColor3f(0.5, 0.25, 0.25);
            } else {
                glColor3f(0, 0, 0);
            }
            drawCenteredRect(p.x, p.y, C_HANDLE_SIZE, C_HANDLE_SIZE);
            glColor4f(ctrl.second.asARGB.r / 255.0, ctrl.second.asARGB.g / 255.0,
                      ctrl.second.asARGB.b / 255.0, ctrl.second.asARGB.a / 255.0);
            drawCenteredRect(p.x, p.y, C_HANDLE_SIZE - C_BORDER_THK, C_HANDLE_SIZE - C_BORDER_THK);
        }
        for (auto &ctrl : nControl) {
            Point p = nCurve.at(ctrl.first);
            if (!selectedP && selected == ctrl.first) {
                glColor3f(0.5, 0.25, 0.25);
            } else {
                glColor3f(0, 0, 0);
            }
            drawCenteredRect(p.x, p.y, C_HANDLE_SIZE, C_HANDLE_SIZE);
            glColor4f(ctrl.second.asARGB.r / 255.0, ctrl.second.asARGB.g / 255.0,
                      ctrl.second.asARGB.b / 255.0, ctrl.second.asARGB.a / 255.0);
            drawCenteredRect(p.x, p.y, C_HANDLE_SIZE - C_BORDER_THK, C_HANDLE_SIZE - C_BORDER_THK);
        }
    }
    glEnd();
}

void ColorCurve::onClick(double x, double y) {
    Point newPoint(x, y);

    for (auto &ctrl : pControl) {
        Point ctrlPoint = pCurve.at(ctrl.first);
        if (ctrlPoint.sqdist(newPoint) < C_SELECTION_RADIUS * C_SELECTION_RADIUS) {
            selected = ctrl.first;
            selectedP = true;
            return;
        }
    }

    for (auto &ctrl : nControl) {
        Point ctrlPoint = nCurve.at(ctrl.first);
        if (ctrlPoint.sqdist(newPoint) < C_SELECTION_RADIUS * C_SELECTION_RADIUS) {
            selected = ctrl.first;
            selectedP = false;
            return;
        }
    }

    std::pair<Point, double> pClosest = pCurve.closestSample(newPoint);
    std::pair<Point, double> nClosest = nCurve.closestSample(newPoint);
    double d1 = pClosest.first.sqdist(newPoint);
    double d2 = nClosest.first.sqdist(newPoint);

    if (d1 < d2 && d1 < C_SELECTION_RADIUS * C_SELECTION_RADIUS) {
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
    } else if (d2 < C_SELECTION_RADIUS * C_SELECTION_RADIUS) {
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
        if (ctrlPoint.sqdist(newPoint) < C_SELECTION_RADIUS * C_SELECTION_RADIUS) {
            pControl.erase(ctrl.first);
            return;
        }
    }

    for (auto &ctrl : nControl) {
        Point ctrlPoint = nCurve.at(ctrl.first);
        if (ctrlPoint.sqdist(newPoint) < C_SELECTION_RADIUS * C_SELECTION_RADIUS) {
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
    return -1;
}

/**
 * Bresenham's line algorithm
 * Psudocode Source: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 */
void drawLine(Eigen::MatrixXi &surface, Point &corner, Point &p1, Point &p2, int val) {
    int x0 = (int) (p1.x-corner.x), x1 = (int) (p2.x-corner.x), y0 = (int) (p1.y-corner.y), y1 = (int) (p2.y-corner.y);

    int dx = abs(x1-x0);
    int sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0);
    int sy = y0<y1 ? 1 : -1;
    int err = dx+dy;
    while (true) {
        surface(x0, y0) = val;
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2*err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ColorCurve::renderToMatrix(Eigen::SparseMatrix<double> &data, size_t width, size_t height) {
    data.resize(width * height, 3);
    std::map<int, std::vector<double>> dups;
    std::vector<Tripletd> matList;
    std::set<int> intersections;
    {
        std::vector<double> r, g, b;
        pCurve.interp(pControl, extractRed, r);
        pCurve.interp(pControl, extractGreen, g);
        pCurve.interp(pControl, extractBlue, b);
        pCurve.renderToArray(r, width, height, index, 0, dups, matList, width * height, intersections);
        pCurve.renderToArray(g, width, height, index, 1, dups, matList, width * height, intersections);
        pCurve.renderToArray(b, width, height, index, 2, dups, matList, width * height, intersections);
    }
    {
        std::vector<double> r, g, b;
        nCurve.interp(nControl, extractRed, r);
        nCurve.interp(nControl, extractGreen, g);
        nCurve.interp(nControl, extractBlue, b);
        nCurve.renderToArray(r, width, height, index, 0, dups, matList, width * height, intersections);
        nCurve.renderToArray(g, width, height, index, 1, dups, matList, width * height, intersections);
        nCurve.renderToArray(b, width, height, index, 2, dups, matList, width * height, intersections);
    }
    dups.clear();
    int segment = 0;
    for (int i = 0; i < samples.size()-1; i ++) {
        while (segment + 1 < segments.size() && segments.at(segment) <= i && i < segments.at(segment+1))
            segment ++;
        if (voidSegments.find(segment) != voidSegments.end())
            continue;
        Point &p1 = pOffset.at(i);
        Point &p2 = pOffset.at(i+1);
        Point &p3 = nOffset.at(i);
        Point &p4 = nOffset.at(i+1);

        Point corner(
                (int) MIN(MIN(MIN(p1.x, p2.x), p3.x), p4.x),
                (int) MIN(MIN(MIN(p1.y, p2.y), p3.y), p4.y)
                );
        Point other_corner(
                (int) MAX(MAX(MAX(p1.x, p2.x), p3.x), p4.x),
                (int) MAX(MAX(MAX(p1.y, p2.y), p3.y), p4.y)
        );
        Eigen::MatrixXi flood_map = Eigen::MatrixXi::Zero(ceil(other_corner.x - corner.x)+1, ceil(other_corner.y - corner.y)+1);
        drawLine(flood_map, corner, p1, p3, 2);
        drawLine(flood_map, corner, p2, p4, 2);
        drawLine(flood_map, corner, p1, p2, 1);
        drawLine(flood_map, corner, p3, p4, 1);

        Point start((int) ((p1.x + p2.x + p3.x + p4.x) / 4), (int) ((p1.y + p2.y + p3.y + p4.y) / 4));
        flood_fill(start, flood_map, corner, dups, width, height);
    }
    Curve::finalizeArrayRender(data, colordup, width, height, dups, matList, width * height, [](const int& row, const int& col, const double & value){
        return 0 <= value && value <= 1;
    });
}

size_t indexDx(size_t x, size_t y, size_t width, size_t height) {
    return y * width + x;
}

size_t indexDy(size_t x, size_t y, size_t width, size_t height) {
    return (width * height) + (y * width + x);
}

double normdup(const std::vector<double> &dups) {
    double sum = 0;
    for (double d : dups)
        sum += d;
    return sum / dups.size();
}

void ColorCurve::renderNormToMatrix(Eigen::SparseMatrix<double> &data, size_t width, size_t height) {
    data.resize(width * height * 2, 3);
    std::map<int, std::vector<double>> dups;
    std::vector<Tripletd> matList;
    Curve curve(samples, segments, voidSegments);
    std::set<int> intersections;
    {
        std::vector<double> pr, pg, pb, nr, ng, nb;
        pCurve.interp(pControl, extractRed, pr);
        pCurve.interp(pControl, extractGreen, pg);
        pCurve.interp(pControl, extractBlue, pb);
        nCurve.interp(nControl, extractRed, nr);
        nCurve.interp(nControl, extractGreen, ng);
        nCurve.interp(nControl, extractBlue, nb);

        std::vector<double> rdx, rdy, gdx, gdy, bdx, bdy;
        for (int i = 0; i < norms.size(); i++) {
            Point &norm = norms.at(i);
            rdx.emplace_back(norm.x * (pr.at(i) - nr.at(i)));
            gdx.emplace_back(norm.x * (pg.at(i) - ng.at(i)));
            bdx.emplace_back(norm.x * (pb.at(i) - nb.at(i)));
            rdy.emplace_back(norm.y * (pr.at(i) - nr.at(i)));
            gdy.emplace_back(norm.y * (pg.at(i) - ng.at(i)));
            bdy.emplace_back(norm.y * (pb.at(i) - nb.at(i)));
        }

        curve.renderToArray(rdx, width, height, indexDx, 0, dups, matList, width * height * 2, intersections);
        curve.renderToArray(gdx, width, height, indexDx, 1, dups, matList, width * height * 2, intersections);
        curve.renderToArray(bdx, width, height, indexDx, 2, dups, matList, width * height * 2, intersections);
        curve.renderToArray(rdy, width, height, indexDy, 0, dups, matList, width * height * 2, intersections);
        curve.renderToArray(gdy, width, height, indexDy, 1, dups, matList, width * height * 2, intersections);
        curve.renderToArray(bdy, width, height, indexDy, 2, dups, matList, width * height * 2, intersections);
    }
    Curve::finalizeArrayRender(data, normdup, width, height, dups, matList, width * height * 2, nullptr);
}

void ColorCurve::flood_fill(Point start, Eigen::MatrixXi &map, Point corner, std::map<int, std::vector<double>> &out,
                            size_t width, size_t height) {
    if (map.coeff((int)(start.x-corner.x), (int)(start.y-corner.y)) == 1)
        return;
    map.coeffRef((int)(start.x-corner.x), (int)(start.y-corner.y)) = 1;
    int start_index = index(start.x, start.y, width, height);
    for (int c = 0; c < 3; c ++) {
        if (out.find((c*width*height) + start_index) == out.end())
            out.emplace((c*width*height) + start_index, std::vector<double>{});
        out.at((c*width*height) + start_index).emplace_back(0);
        out.at((c*width*height) + start_index).emplace_back(0);
    }
    std::queue<Point> Q;
    Q.emplace(start.x-corner.x, start.y-corner.y);
    while (!Q.empty()) {
        Point n = Q.front();
        Q.pop();
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++)
                if (abs(dx) + abs(dy) == 1) {
                    if (n.x + dx >= 0 && n.y + dy >= 0 && n.x + dx < map.rows() && n.y + dy < map.cols()) {
                        int idx = index(n.x + dx + corner.x, n.y + dy + corner.y, width, height);
                        if (map.coeff((int) (n.x + dx), (int) (n.y + dy)) != 1) {
                            bool cont = map.coeff((int) (n.x + dx), (int) (n.y + dy)) == 0;
                            map.coeffRef((int) (n.x + dx), (int) (n.y + dy)) = 1;
                            for (int c = 0; c < 3; c++) {
                                if (out.find((c * width * height) + idx) == out.end())
                                    out.emplace((c * width * height) + idx, std::vector<double>{});
                                out.at((c * width * height) + idx).emplace_back(0);
                                out.at((c * width * height) + idx).emplace_back(0);
                            }
                            if (cont)
                                Q.emplace(n.x + dx, n.y + dy);
                        }
                    }
                }
        }
    }
}

ARGBInt::ARGBInt(int i) : asInt(i) {
}

ARGBInt::ARGBInt(ubyte a, ubyte r, ubyte g, ubyte b) : asARGB({b, g, r, a}) {
}

ARGBInt::ARGBInt(double a, double r, double g, double b) :
        asARGB({static_cast<ubyte>(((int) (b * 255)) & 0xFF), static_cast<ubyte>(((int) (g * 255)) & 0xFF),
                static_cast<ubyte>(((int) (r * 255)) & 0xFF), static_cast<ubyte>(((int) (a * 255)) & 0xFF)}) {
}

double extractRed(ARGBInt argb) {
    return argb.asARGB.r / 255.0;
}

double extractGreen(ARGBInt argb) {
    return argb.asARGB.g / 255.0;
}

double extractBlue(ARGBInt argb) {
    return argb.asARGB.b / 255.0;
}