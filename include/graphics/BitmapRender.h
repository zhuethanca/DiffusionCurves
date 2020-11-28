#pragma once
#include <Eigen/Eigen>
#include "ColorCurve.h"

class BitmapRender {
public:
    void setData(Eigen::SparseMatrix<double> &data, size_t width, size_t height,
                               size_t (*index)(size_t, size_t, size_t, size_t));
    void setData(Eigen::MatrixXd &data, size_t width, size_t height,
    size_t (*index)(size_t, size_t, size_t, size_t));
    void render();
    ~BitmapRender();
private:
    ubyte* data = nullptr;
    size_t width;
    size_t height;
};
