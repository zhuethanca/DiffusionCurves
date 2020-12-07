#include "graphics/SolveWrappers.h"

#include "diffusion/rasterize_color.h"
#include "diffusion/rasterize_norms.h"
#include "diffusion/rasterize_gaussian.h"
#include "diffusion/solve_diffusion.h"
#include "diffusion/solve_gaussian.h"

void solve_diffusion(Bezier &bezier, ColorCurve &colorCurve,
                     size_t width, size_t height, Eigen::MatrixXd &rgbImage) {
    Eigen::SparseMatrix<double> rgb;
    rasterize_color(bezier.segments, bezier.voidSegments,
                    bezier.pOffset, bezier.nOffset,
                    colorCurve.pCurve.controlToIndex(colorCurve.pControl),
                    colorCurve.nCurve.controlToIndex(colorCurve.nControl),
                    rgb, width, height);

    Eigen::SparseMatrix<double> norms;
    rasterize_norms(bezier.samples, bezier.norms,
                    bezier.segments, bezier.voidSegments,
                    bezier.pOffset, bezier.nOffset,
                    colorCurve.pCurve.controlToIndex(colorCurve.pControl),
                    colorCurve.nCurve.controlToIndex(colorCurve.nControl),
                    norms, width, height);
    std::cout << "Solving..." << std::endl;

    solve_diffusion(rgb, norms, width, height, rgbImage);
}

void solve_gaussian(Bezier &bezier, GaussianCurve &gaussianCurve,
                    size_t width, size_t height, Eigen::MatrixXd &blurImage) {
    Eigen::SparseMatrix<double> blur;
    rasterize_gaussian(bezier.samples, bezier.norms,
                       bezier.segments, bezier.voidSegments,
                       gaussianCurve.curve.controlToIndex(gaussianCurve.control),
                       blur, width, height);

    std::cout << "Solving..." << std::endl;
    solve_gaussian(blur, width, height, blurImage);
}