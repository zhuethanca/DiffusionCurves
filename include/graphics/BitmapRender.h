#pragma once
#include <Eigen/Eigen>
#include <opencv2/core/mat.hpp>
#include "ColorCurve.h"
#include <memory>


class BitmapRender {
public:
    void setData(Eigen::SparseMatrix<double> &data, size_t width, size_t height,
                 size_t (*index)(size_t, size_t, size_t, size_t));
    void setGaussData(Eigen::SparseMatrix<double> &data, size_t width, size_t height,
                 size_t (*index)(size_t, size_t, size_t, size_t));
    void setData(Eigen::MatrixXd &data, size_t width, size_t height,
                 size_t (*index)(size_t, size_t, size_t, size_t));
    void setGaussData(Eigen::MatrixXd &data, size_t width, size_t height,
                 size_t (*index)(size_t, size_t, size_t, size_t));
    void setData(cv::Mat& data);
    cv::Mat getData();

    void render();

private:
    std::vector<ubyte> data;
    size_t width;
    size_t height;
};
