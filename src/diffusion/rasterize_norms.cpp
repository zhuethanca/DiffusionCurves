#include "diffusion/rasterize_norms.h"
#include "util/util.h"
#include "diffusion/interpolate_control.h"
#include "diffusion/rasterize_samples.h"

void rasterize_norms(std::vector<Point> &samples, std::vector<Point> &norms,
                     std::vector<int> &segments, std::set<int> &voidSegments,
                     std::vector<Point> &pOffset, std::vector<Point> &nOffset,
                     const std::map<size_t, ARGBInt>& pControl, const std::map<size_t, ARGBInt>& nControl,
                     Eigen::SparseMatrix<double> &data, size_t width, size_t height) {
    data.resize(width * height * 2, 3);
    std::map<int, std::vector<double>> dups;
    std::vector<Tripletd> matList;
    std::set<int> intersections;
    {
        std::vector<double> pr, pg, pb, nr, ng, nb;
        interpolate_control(pOffset, pControl, extractRed, pr);
        interpolate_control(pOffset, pControl, extractGreen, pg);
        interpolate_control(pOffset, pControl, extractBlue, pb);
        interpolate_control(nOffset, nControl, extractRed, nr);
        interpolate_control(nOffset, nControl, extractGreen, ng);
        interpolate_control(nOffset, nControl, extractBlue, nb);

        std::vector<double> rdx, rdy, gdx, gdy, bdx, bdy;
        for (int i = 0; i < norms.size(); i++) {
            Point &norm = norms.at(i);
            rdx.emplace_back(norm.x * (pr.at(i) - nr.at(i)));
            gdx.emplace_back(norm.x * (pg.at(i) - ng.at(i)));
            bdx.emplace_back(norm.x * (pb.at(i) - nb.at(i)));
            rdy.emplace_back(norm.y * (pr.at(i) - nr.at(i)));
            gdy.emplace_back(norm.y * (pg.at(i) - ng.at(i)));
            bdy.emplace_back(norm.y * (pb.at(i) - nb.at(i)));
        }
        std::vector<Tripletd> tmpMatList;
        rasterize_samples(samples, segments, voidSegments, rdx, width, height, indexDx, 0, tmpMatList);
        rasterize_samples(samples, segments, voidSegments, gdx, width, height, indexDx, 1, tmpMatList);
        rasterize_samples(samples, segments, voidSegments, bdx, width, height, indexDx, 2, tmpMatList);
        std::copy_if (tmpMatList.begin(), tmpMatList.end(), std::back_inserter(matList), [width, height](Tripletd d){
            return 0 <= d.row() && d.row() < width*height && 0 <= d.col() && d.col() < 3;
        } );
        tmpMatList.clear();
        rasterize_samples(samples, segments, voidSegments, rdy, width, height, indexDy, 0, tmpMatList);
        rasterize_samples(samples, segments, voidSegments, gdy, width, height, indexDy, 1, tmpMatList);
        rasterize_samples(samples, segments, voidSegments, bdy, width, height, indexDy, 2, tmpMatList);
        std::copy_if (tmpMatList.begin(), tmpMatList.end(), std::back_inserter(matList), [width, height](Tripletd d){
            return width*height <= d.row() && d.row() < 2*width*height && 0 <= d.col() && d.col() < 3;
        });
    }
    data.setFromTriplets(matList.begin(), matList.end(), [] (const double &,const double &b) { return 0; });
}
