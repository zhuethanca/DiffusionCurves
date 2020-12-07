#pragma once
#include <map>
#include <vector>

/**
 * Given a vector of control points, containing some data T,
 * interpolate the data extracted using extract(T) along the samples
 */
template<class T>
void interpolate_control(std::vector<Point>& samples,
            const std::map<size_t, T>& control, double (*extract) (T),
            std::vector<double>& data) {
    data.clear();
    if (control.empty()) {
        for (int i = 0; i < samples.size(); i ++)
            data.push_back(0);
        return;
    }
    auto iter = control.begin();
    int last = (*iter).first;
    double lastVal = extract((*iter).second);
    for (int i = 0; i < last; i ++) {
        data.push_back(lastVal);
    }
    for(iter++;iter != control.end(); iter ++) {
        int curr = (*iter).first;
        double val = extract((*iter).second);

        double* diff = new double[curr-last+1];
        diff[0] = 0;
        for (int i = last+1; i <= curr; i ++) {
            diff[i-last] = diff[i-last-1] + samples.at(i-1).dist(samples.at(i));
        }
        double len = diff[curr-last];
        for (int i = 0; i < curr-last; i ++) {
            double t = diff[i]/len;
            data.push_back(val*t + (1-t)*lastVal);
        }

        last = curr;
        lastVal = val;
    }
    for (int i = last; i < samples.size(); i ++) {
        data.push_back(lastVal);
    }
}