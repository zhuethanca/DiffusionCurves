//
// Created by Ethan on 11/23/2020.
//

#include <cmath>
#include "graphics/Point.h"

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

double Point::norm() {
    return sqrt(x*x + y*y);
}
