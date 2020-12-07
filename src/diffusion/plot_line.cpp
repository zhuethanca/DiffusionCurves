#include "diffusion/plot_line.h"


void plot_line(Point &p1, Point &p2, double d1, double d2,
                       size_t width, size_t height, size_t (*index)(size_t x, size_t y, size_t width, size_t height),
                       std::vector<Tripletd> &target, size_t col) {

    int x1 = (int) p1.x, x2 = (int) p2.x, y1 = (int) p1.y, y2 = (int) p2.y;
    int dx = abs(x2-x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2-y1);
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;

    double length = p1.dist(p2);

    while (true) {
        Point p(x1, y1);
        if (0 <= p.x && p.x < width && 0 <= p.y && p.y < height) {
            double t = p.dist(p1) / length;
            double d = d2 * t + (1 - t) * d1;
            uint32_t idx = index(p.x, p.y, width, height);
            target.emplace_back(idx, col, d);
        }

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