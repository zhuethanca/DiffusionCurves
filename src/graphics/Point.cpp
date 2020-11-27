//
// Created by Ethan on 11/23/2020.
//

#include <cmath>
#include "graphics/Point.h"
#include <iostream>

Point::Point() : Point(0, 0) {
}

Point::Point(double x, double y) {
    this->x = x;
    this->y = y;
}

double Point::dist(const Point o) const {
    return sqrt((x-o.x)*(x-o.x) + (y-o.y)*(y-o.y));
}

void Point::normalize() {
    double norm = this->norm();
    if (norm != 0) {
        x /= norm;
        y /= norm;
    }
}

double Point::norm() const {
    return sqrt(x*x + y*y);
}

double Point::sqdist(Point o) const {
    return (x-o.x)*(x-o.x) + (y-o.y)*(y-o.y);
}

bool Point::operator==(const Point &rhs) const {
    return x == rhs.x &&
           y == rhs.y;
}

bool Point::operator!=(const Point &rhs) const {
    return !(rhs == *this);
}

std::ostream& std::operator<<(std::ostream &strm, const Point &p) {
    return strm << "(" << p.x << ", " << p.y << ")";
}