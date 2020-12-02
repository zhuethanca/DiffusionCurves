#pragma once
#include <Eigen/Eigen>
#include "ColorCurve.h"
#include <memory>

class BitmapRender {
public:
    void setData(Eigen::SparseMatrix<double> &data, size_t width, size_t height,
                               size_t (*index)(size_t, size_t, size_t, size_t));
    void setData(Eigen::MatrixXd &data, size_t width, size_t height,
    size_t (*index)(size_t, size_t, size_t, size_t));
    void render();
private:
    std::vector<ubyte> data;
    size_t width;
    size_t height;
};
