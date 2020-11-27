#pragma once
#include <vector>
#include "graphics/Point.h"
#include <utility>
#include <map>

class Curve {
public:
    explicit Curve(std::vector<Point> &samples, std::vector<int>& segments);
    void render();
    void render(const std::vector<double>& r, const std::vector<double>& g, const std::vector<double>& b);
    std::pair<Point, double> closestSample(Point& target);
    void offset(const std::vector<Point>& norms, double dist, std::vector<Point>& res) const;
    Point at(double factor);
    size_t atIndex(double factor);
    double length() const;
    template<class T>
    void interp(const std::map<double, T>& control, double (*extract) (T), std::vector<double>& res) {
        res.clear();
        if (control.empty()) {
            for (int i = 0; i < samples.size(); i ++)
                res.push_back(0);
            return;
        }
        auto iter = control.begin();
        int last = atIndex((*iter).first);
        double lastVal = extract((*iter).second);
        for (int i = 0; i <= last; i ++) {
            res.push_back(lastVal);
        }
        for(iter++;iter != control.end(); iter ++) {
            int curr = atIndex((*iter).first);
            double val = extract((*iter).second);

            double diff[curr-last+1];
            diff[0] = 0;
            for (int i = last+1; i <= curr; i ++) {
                diff[i-last] = diff[i-last-1] + samples.at(i-1).dist(samples.at(i));
            }
            double len = diff[curr-last];
            for (int i = 0; i < curr-last; i ++) {
                double t = diff[i]/len;
                res.push_back(val*t + (1-t)*lastVal);
            }

            last = curr;
            lastVal = val;
        }
        for (int i = last; i < samples.size(); i ++) {
            res.push_back(lastVal);
        }
    }
private:
    std::vector<Point>& samples;
    std::vector<int>& segments;
    double computeSegmentPos(double t);
};
