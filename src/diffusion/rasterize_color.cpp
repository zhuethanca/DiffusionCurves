#include "diffusion/rasterize_color.h"
#include "diffusion/rasterize_samples.h"
#include "diffusion/interpolate_control.h"
#include <queue>
#include <unordered_set>

/**
 * Pair Hash Code: https://www.techiedelight.com/use-std-pair-key-std-unordered_map-cpp/
 */
struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

void plot_line(Eigen::MatrixXi &surface, Point &corner, Point &p1, Point &p2, int val);
void flood_fill(Point start, Eigen::MatrixXi &map, Point corner, std::unordered_set<std::pair<int, int>, pair_hash> &out,
                size_t width, size_t height);

void rasterize_color(std::vector<int> &segments, std::set<int> &voidSegments,
                    std::vector<Point> &pOffset, std::vector<Point> &nOffset,
                    const std::map<size_t, ARGBInt>& pControl, const std::map<size_t, ARGBInt>& nControl,
                    Eigen::SparseMatrix<double> &data, size_t width, size_t height) {
    data.resize(width * height, 3);
    std::vector<Tripletd> matList;
    {
        std::vector<double> r, g, b;
        interpolate_control(pOffset, pControl, extractRed, r);
        interpolate_control(pOffset, pControl, extractGreen, g);
        interpolate_control(pOffset, pControl, extractBlue, b);
        rasterize_samples(pOffset, segments, voidSegments, r, width, height, index, 0, matList);
        rasterize_samples(pOffset, segments, voidSegments, g, width, height, index, 1, matList);
        rasterize_samples(pOffset, segments, voidSegments, b, width, height, index, 2, matList);
    }
    {
        std::vector<double> r, g, b;
        interpolate_control(nOffset, nControl, extractRed, r);
        interpolate_control(nOffset, nControl, extractGreen, g);
        interpolate_control(nOffset, nControl, extractBlue, b);
        rasterize_samples(nOffset, segments, voidSegments, r, width, height, index, 0, matList);
        rasterize_samples(nOffset, segments, voidSegments, g, width, height, index, 1, matList);
        rasterize_samples(nOffset, segments, voidSegments, b, width, height, index, 2, matList);
    }

    std::unordered_set<std::pair<int, int>, pair_hash> dups;
    int segment = 0;
    for (int i = 0; i < pOffset.size()-1; i ++) {
        while (segment + 1 < segments.size() && segments.at(segment) <= i && i < segments.at(segment+1))
            segment ++;
        if (voidSegments.find(segment) != voidSegments.end())
            continue;
        Point &p1 = pOffset.at(i);
        Point &p2 = pOffset.at(i+1);
        Point &p3 = nOffset.at(i);
        Point &p4 = nOffset.at(i+1);

        Point corner(
                (int) MIN(MIN(MIN(p1.x, p2.x), p3.x), p4.x),
                (int) MIN(MIN(MIN(p1.y, p2.y), p3.y), p4.y)
        );
        Point other_corner(
                (int) MAX(MAX(MAX(p1.x, p2.x), p3.x), p4.x),
                (int) MAX(MAX(MAX(p1.y, p2.y), p3.y), p4.y)
        );
        Eigen::MatrixXi flood_map = Eigen::MatrixXi::Zero(ceil(other_corner.x - corner.x)+1, ceil(other_corner.y - corner.y)+1);
        plot_line(flood_map, corner, p1, p3, 2);
        plot_line(flood_map, corner, p2, p4, 2);
        plot_line(flood_map, corner, p1, p2, 1);
        plot_line(flood_map, corner, p3, p4, 1);

        Point start((int) ((p1.x + p2.x + p3.x + p4.x) / 4), (int) ((p1.y + p2.y + p3.y + p4.y) / 4));
        flood_fill(start, flood_map, corner, dups, width, height);
    }

    for (const auto& pair : dups) {
        matList.emplace_back(pair.first, pair.second, -1);
    }
    std::vector<Tripletd> cleanMatList(matList.size());
    std::copy_if (matList.begin(), matList.end(), std::back_inserter(cleanMatList), [width, height](Tripletd d){
        return 0 <= d.row() && d.row() < width*height && 0 <= d.col() && d.col() < 3;
    } );

    data.setFromTriplets(cleanMatList.begin(), cleanMatList.end(), [] (const double &,const double &b) { return b; });

    data.prune([](const int& row, const int& col, const double & value){
        return 0 <= value && value <= 1;
    });
}

/**
 * Bresenham's line algorithm
 * Psudocode Source: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
 *
 * Same algorithm as in plot_line.h, but with slightly different interface.
 */
void plot_line(Eigen::MatrixXi &surface, Point &corner, Point &p1, Point &p2, int val) {
    int x0 = (int) (p1.x-corner.x), x1 = (int) (p2.x-corner.x), y0 = (int) (p1.y-corner.y), y1 = (int) (p2.y-corner.y);

    int dx = abs(x1-x0);
    int sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0);
    int sy = y0<y1 ? 1 : -1;
    int err = dx+dy;
    while (true) {
        surface(x0, y0) = val;
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2*err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

/**
 * Runs the flood fill algorithm. Starts filling from start, spreading to grid values of 0. It is bounded
 * by non-zero values. Boundary values that is not 1 is overwritten.
 */
void flood_fill(Point start, Eigen::MatrixXi &map, Point corner, std::unordered_set<std::pair<int, int>, pair_hash> &out,
                            size_t width, size_t height) {
    if (map.coeff((int)(start.x-corner.x), (int)(start.y-corner.y)) == 1)
        return;
    map.coeffRef((int)(start.x-corner.x), (int)(start.y-corner.y)) = 1;
    int start_index = index(start.x, start.y, width, height);
    for (int c = 0; c < 3; c ++) {
        out.emplace(start_index, c);
    }
    std::queue<Point> Q;
    Q.emplace(start.x-corner.x, start.y-corner.y);
    while (!Q.empty()) {
        Point n = Q.front();
        Q.pop();
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++)
                if (abs(dx) + abs(dy) == 1) {
                    if (n.x + dx >= 0 && n.y + dy >= 0 && n.x + dx < map.rows() && n.y + dy < map.cols()) {
                        int idx = index(n.x + dx + corner.x, n.y + dy + corner.y, width, height);
                        if (map.coeff((int) (n.x + dx), (int) (n.y + dy)) != 1) {
                            bool cont = map.coeff((int) (n.x + dx), (int) (n.y + dy)) == 0;
                            map.coeffRef((int) (n.x + dx), (int) (n.y + dy)) = 1;
                            for (int c = 0; c < 3; c++) {
                                out.emplace(idx, c);
                            }
                            if (cont)
                                Q.emplace(n.x + dx, n.y + dy);
                        }
                    }
                }
        }
    }
}