#include "graphics/Curve.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include "diffusion/plot_line.h"

Curve::Curve(std::vector<Point> &samples, std::vector<int> &segments, std::set<int> &voidSegments)
        : samples(samples), segments(segments), voidSegments(voidSegments) {

}

void Curve::render() {
    glBegin(GL_LINES);{
        glColor3f(0, 0, 0);
        int segment = 0;

        int sampleSize = samples.size();
        for (int i = 0; i < sampleSize - 1; i++) {
            while (segment + 1 < segments.size() && segments.at(segment) <= i && i < segments.at(segment+1))
                segment ++;
            if (voidSegments.find(segment) != voidSegments.end())
                continue;
            Point &p1 = samples.at(i);
            Point &p2 = samples.at(i+1);
            glVertex2f(p1.x, p1.y);
            glVertex2f(p2.x, p2.y);
        }
    }glEnd();
}

void Curve::render(const std::vector<double>& r, const std::vector<double>& g, const std::vector<double>& b) {
    glBegin(GL_LINES);{
        int segment = 0;
        for (int i = 0; i+1 < samples.size(); i ++) {
            while (segment + 1 < segments.size() && segments.at(segment) <= i && i < segments.at(segment+1))
                segment ++;
            if (voidSegments.find(segment) != voidSegments.end())
                continue;
            Point &p1 = samples.at(i);
            Point &p2 = samples.at(i + 1);
            glColor3f(r.at(i), g.at(i), b.at(i));
            glVertex2f(p1.x, p1.y);
            glColor3f(r.at(i + 1), g.at(i + 1), b.at(i + 1));
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

//void Curve::finalizeArrayRender(Eigen::SparseMatrix<double> &target, double (*dupHandler)(const std::vector<double>&),
//                                size_t width, size_t height,
//                                std::map<int, std::vector<double>> &dups, std::vector<Tripletd> &matList, int max,
//                                bool (*keepFunc)(const int & row, const int & col, const double & value)) {
//    target.setFromTriplets(matList.begin(), matList.end(), [] (const double &,const double &b) { return b; });
//
//    for (const auto& pair : dups) {
//        if (pair.second.size() > 1) {
//            target.coeffRef(pair.first % (max), pair.first / (max)) = dupHandler(pair.second);
//        }
//    }
//
//    if (keepFunc != nullptr)
//        target.prune(keepFunc);
//}