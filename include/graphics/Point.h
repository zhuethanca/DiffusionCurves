//
// Created by Ethan on 11/23/2020.
//
#pragma once

class Point {
public:
    Point();
    Point(double x, double y);
    double dist(Point o) const;
    double x;
    double y;

    double norm();
    void normalize();
};
