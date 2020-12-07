#pragma once
#include <vector>
#include "graphics/Point.h"
#include "util/util.h"
#include <map>
#include <set>
#include <Eigen/Eigen>

class Curve {
public:
    explicit Curve(std::vector<Point> &samples, std::vector<int> &segments, std::set<int> &voidSegments);
    void render();
    void render(const std::vector<double>& r, const std::vector<double>& g, const std::vector<double>& b);
    std::pair<Point, double> closestSample(Point& target);
    void offset(const std::vector<Point>& norms, double dist, std::vector<Point>& res) const;
    Point at(double factor);
    size_t atIndex(double factor);
    double length() const;

    template <class T>
    std::map<size_t, T> controlToIndex(std::map<double, T> &controls) {
        std::map<size_t, T> res;
        for (auto &ctrl : controls) {
            res.emplace(atIndex(ctrl.first), ctrl.second);
        }
        return res;
    }
private:
    std::vector<Point>& samples;
    std::vector<int>& segments;
    std::set<int>& voidSegments;
    double computeSegmentPos(double t);
};
