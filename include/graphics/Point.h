//
// Created by Ethan on 11/23/2020.
//
#pragma once
#include <iostream>
#include <Eigen/Core>
#include <functional>

class Point {
public:
    Point();
    Point(double x, double y);
    double dist(Point o) const;
    double sqdist(Point o) const;
    Eigen::Vector2f toVector();
    double x;
    double y;

    double norm() const;
    void normalize();
    bool isNeighbour(const Point other);

    bool operator==(const Point &rhs) const;
    bool operator!=(const Point &rhs) const;
};

namespace std {
    template <>
    struct hash<Point>
    {
        size_t operator()(const Point& k) const
        {
            // Compute individual hash values for two data members and combine them using XOR and bit shifting
            return ((hash<double>()(k.x)) ^ (hash<double>()(k.y)) >> 1);
        }
    };

    std::ostream& operator<<(std::ostream &strm, const Point &p);
}