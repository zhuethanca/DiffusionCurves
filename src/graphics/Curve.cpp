#include "graphics/Curve.h"
#include <GL/glew.h>
#include <cmath>

Curve::Curve(std::vector<Point> &samples, std::vector<int>& segments) : samples(samples), segments(segments){

}

void Curve::render() {
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

void Curve::render(const std::vector<double>& r, const std::vector<double>& g, const std::vector<double>& b) {
    glBegin(GL_LINES);{
        for (int i = 1; i < samples.size(); i ++) {
            Point &p1 = samples.at(i - 1);
            Point &p2 = samples.at(i);
            glColor3f(r.at(i-1), g.at(i-1), b.at(i-1));
            glVertex2f(p1.x, p1.y);
            glColor3f(r.at(i), g.at(i), b.at(i));
            glVertex2f(p2.x, p2.y);
        }
    }glEnd();
}

std::pair<Point, double> Curve::closestSample(Point &target) {
    double mdst = INFINITY;
    Point res = Point(INFINITY, INFINITY);
    double len = 0;
    double mlen = 0;
    for (int i = 0; i < samples.size(); i ++) {
        Point &p = samples.at(i);
        if (i > 0) {
            len += p.dist(samples.at(i-1));
        }
        if (p.sqdist(target) < mdst) {
            mdst = p.sqdist(target);
            res = p;
            mlen = len;
        }
    }
    return std::make_pair(res, computeSegmentPos(mlen/length()));
}

void Curve::offset(const std::vector<Point>& norms, double dist, std::vector<Point> &res) const {
    res.clear();
    for (int i = 0; i < samples.size(); i ++) {
        const Point &p = samples.at(i);
        const Point &n = norms.at(i);
        res.emplace_back(p.x + n.x * dist, p.y + n.y * dist);
    }
}

double Curve::length() const {
    double len = 0;
    for (int i = 1; i < samples.size(); i ++) {
        const Point &n = samples.at(i-1);
        const Point &p = samples.at(i);
        len += n.dist(p);
    }
    return len;
}

Point Curve::at(double factor) {
    int seg = floor(factor);
    double residual = factor - seg;
    if (seg + 1 >= segments.size()) {
        return samples.at(samples.size()-1);
    }

    double cum = 0;
    double segLength = 0;
    for (int i = segments.at(seg); i < segments.at(seg+1); i ++) {
        segLength += samples.at(i+1).dist(samples.at(i));
    }
    for (int i = segments.at(seg); i < segments.at(seg+1); i ++) {
        const Point &n = samples.at(i);
        const Point &p = samples.at(i+1);
        double len = n.dist(p);
        if (cum/segLength <= residual && residual < (cum+len)/segLength) {
            double t = (residual*segLength - cum)/(len);
            double dx = (p.x - n.x) * t;
            double dy = (p.y - n.y) * t;
            return {n.x + dx, n.y + dy};
        }
        cum += len;
    }
    return {};
}

size_t Curve::atIndex(double factor) {
    int seg = floor(factor);
    double residual = factor - seg;
    if (seg + 1 >= segments.size()) {
        return samples.size()-1;
    }

    double cum = 0;
    double segLength = 0;
    for (int i = segments.at(seg); i < segments.at(seg+1); i ++) {
        segLength += samples.at(i+1).dist(samples.at(i));
    }
    for (int i = segments.at(seg); i < segments.at(seg+1); i ++) {
        const Point &n = samples.at(i);
        const Point &p = samples.at(i+1);
        double len = n.dist(p);
        if (cum/segLength <= residual && residual < (cum+len)/segLength) {
            return i;
        }
        cum += len;
    }
    return 0;
}

double Curve::computeSegmentPos(double factor) {
    double cum = 0;
    double segCum = 0;
    double total = length();
    int seg = 0;
    double residual = 0;
    for (int i = 1; i < samples.size(); i ++) {
        const Point &n = samples.at(i-1);
        const Point &p = samples.at(i);
        double len = n.dist(p);
        if ((cum+segCum)/total <= factor && factor < (cum+segCum+len)/total) {
            residual = factor*total - cum;
            break;
        }
        segCum += len;
        if (i >= segments.at(seg+1)) {
            seg ++;
            cum += segCum;
            segCum = 0;
        }
    }
    if (seg + 1 >= segments.size())
        return seg;
    double segLength = 0;
    for (int i = segments.at(seg); i < segments.at(seg+1); i ++) {
        segLength += samples.at(i+1).dist(samples.at(i));
    }
    return seg + residual/segLength;
}

void Curve::renderToArray(const std::vector<double> &data, size_t width, size_t height,
                   size_t (*index)(size_t, size_t, size_t, size_t), Eigen::SparseMatrix<double> &target, size_t col,
                   double (*dupHandler)(const std::vector<double>&), int max) {
    std::map<int, std::vector<double>> dups;
    std::vector<Tripletd> matList;
    renderToArray(data, width, height, index, col, dups, matList, max);
    finalizeArrayRender(target, dupHandler, width, height, dups, matList, max);
}

void Curve::renderToArray(const std::vector<double> &data, size_t width, size_t height,
                   size_t (*index)(size_t, size_t, size_t, size_t), size_t col,
                   std::map<int, std::vector<double>> &dups, std::vector<Tripletd> &matList, int max) {

    for (int i = 1; i < samples.size(); i ++) {
        Point& p1 = samples.at(i-1);
        Point& p2 = samples.at(i);
        double d1 = data.at(i-1);
        double d2 = data.at(i);
        renderLine(p1, p2, d1, d2, width, height, index, matList, col, dups, max);
    }
}

void Curve::finalizeArrayRender(Eigen::SparseMatrix<double> &target, double (*dupHandler)(const std::vector<double>&),
                                size_t width, size_t height,
                                std::map<int, std::vector<double>> &dups, std::vector<Tripletd> &matList, int max) {
    target.setFromTriplets(matList.begin(), matList.end());

    for (const auto& pair : dups) {
        if (pair.second.size() > 1)
            target.coeffRef(pair.first % (max), pair.first / (max)) = dupHandler(pair.second);
    }
}

void Curve::renderLine(Point &p1, Point &p2, double d1, double d2,
                       size_t width, size_t height, size_t (*index)(size_t x, size_t y, size_t width, size_t height),
                       std::vector<Tripletd> &target, size_t col, std::map<int, std::vector<double>>& dups, int max) {
    int x1 = (int) p1.x, x2 = (int) p2.x, y1 = (int) p1.y, y2 = (int) p2.y;
    int dx = abs(x2-x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2-y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;

    double length = p1.dist(p2);

    while (true) {
        Point p(x1, y1);
        double t = p.dist(p1) / length;
        double d = d2 * t + (1 - t) * d1;
        uint32_t idx = index(p.x, p.y, width, height);
        target.emplace_back(idx, col, d);
        uint32_t dupidx = (col*max) + idx;

        if (dups.find(dupidx) == dups.end())
            dups.emplace(dupidx, std::vector<double>{});
        dups.at(dupidx).emplace_back(d);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2*err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}