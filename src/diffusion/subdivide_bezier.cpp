#include "diffusion/subdivide_bezier.h"

void subdivideBezier(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4,
                             std::vector<Point>& samples, std::vector<Point>& norms) {
    // Calculate all the mid-points of the line segments
    //----------------------
    double x12   = (x1 + x2) / 2;
    double y12   = (y1 + y2) / 2;
    double x23   = (x2 + x3) / 2;
    double y23   = (y2 + y3) / 2;
    double x34   = (x3 + x4) / 2;
    double y34   = (y3 + y4) / 2;
    double x123  = (x12 + x23) / 2;
    double y123  = (y12 + y23) / 2;
    double x234  = (x23 + x34) / 2;
    double y234  = (y23 + y34) / 2;
    double x1234 = (x123 + x234) / 2;
    double y1234 = (y123 + y234) / 2;

    // Try to approximate the full cubic curve by a single straight line
    //------------------
    double dx = x4-x1;
    double dy = y4-y1;

    double d2 = fabs(((x2 - x4) * dy - (y2 - y4) * dx));
    double d3 = fabs(((x3 - x4) * dy - (y3 - y4) * dx));

    if((d2 + d3)*(d2 + d3) < DISTANCE_TOLERANCE * (dx*dx + dy*dy))
    {
        samples.emplace_back(x1234, y1234);
        norms.emplace_back(y234-y1234, x1234-x234);
        Point& norm = (*norms.rbegin());
        if (abs(norm.x) < 1E-4 && abs(norm.y) < 1E-4) {
            norm.x = (y2-y1);
            norm.y = (x1-x2);
        }
        norm.normalize();
        return;
    }

    // Continue subdivision
    //----------------------
    subdivideBezier(x1, y1, x12, y12, x123, y123, x1234, y1234, samples, norms);
    subdivideBezier(x1234, y1234, x234, y234, x34, y34, x4, y4, samples, norms);
}