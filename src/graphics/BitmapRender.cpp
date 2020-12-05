//
// Created by Ethan on 11/27/2020.
//

#include <GLFW/glfw3.h>
#include "graphics/BitmapRender.h"
#include <iostream>

void BitmapRender::render() {
    if (!this->data.empty())
        glDrawPixels(this->width, this->height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
}

void BitmapRender::setData(Eigen::SparseMatrix<double> &data, size_t width, size_t height,
                           size_t (*index)(size_t, size_t, size_t, size_t)) {
    this->data.clear();
    this->data.resize(width*height*3, 0xFF);
    this->width = width;
    this->height = height;
    for (int k = 0; k < data.outerSize(); ++k) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(data, k); it; ++it) {
            uint32_t index = it.row();
            uint32_t x = index % width;
            uint32_t y = index / width;
            this->data[((height-y-1)*width+x)*3 + it.col()] = (ubyte) (it.value() * 255) & 0xFF;
        }
    }
}

void BitmapRender::setGaussData(Eigen::SparseMatrix<double> &data, size_t width, size_t height,
                           size_t (*index)(size_t, size_t, size_t, size_t)) {
    this->data.clear();
    this->data.resize(width*height*3, 0xFF);
    this->width = width;
    this->height = height;
    double max = data.coeffs().maxCoeff()-0.6;
    for (int k = 0; k < 3; ++k) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(data, 0); it; ++it) {
            uint32_t index = it.row();
            uint32_t x = index % width;
            uint32_t y = index / width;
            this->data[((height-y-1)*width+x)*3 + it.col()] = (ubyte) ((it.value()-0.6)/max * 255) & 0xFF;
        }
    }
}

void BitmapRender::setData(Eigen::MatrixXd &data, size_t width, size_t height,
                           size_t (*index)(size_t, size_t, size_t, size_t)) {
    this->data.clear();
    this->data.resize(width*height*3, 0xFF);
    this->width = width;
    this->height = height;
    for (uint32_t x = 0; x < width; x ++) {
        for (uint32_t y = 0; y < height; y ++) {
            this->data[((height-y-1)*width+x)*3 + 0] = ((ubyte) (data(index(x, y, width, height), 0)*255)) & 0xFF;
            this->data[((height-y-1)*width+x)*3 + 1] = ((ubyte) (data(index(x, y, width, height), 1)*255)) & 0xFF;
            this->data[((height-y-1)*width+x)*3 + 2] = ((ubyte) (data(index(x, y, width, height), 2)*255)) & 0xFF;
        }
    }
}

void BitmapRender::setGaussData(Eigen::MatrixXd &data, size_t width, size_t height,
                                size_t (*index)(size_t, size_t, size_t, size_t)) {
    this->data.clear();
    this->data.resize(width*height*3, 0xFF);
    this->width = width;
    this->height = height;
    double max = data.maxCoeff()-0.6;
    for (uint32_t x = 0; x < width; x ++) {
        for (uint32_t y = 0; y < height; y ++) {
            this->data[((height-y-1)*width+x)*3 + 0] =
            this->data[((height-y-1)*width+x)*3 + 1] =
            this->data[((height-y-1)*width+x)*3 + 2] = ((ubyte) (((data(index(x, y, width, height), 0)-0.6)/max)*255)) & 0xFF;
        }
    }
}


void BitmapRender::setData(cv::Mat &data) {
    const int width = data.cols;
    const int height = data.rows;

    this->data.clear();
    this->data.resize(width * height * 3, 0xFF);
    this->width = width;
    this->height = height;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            this->data[((height - y - 1) * width + x) * 3 + 0] = (ubyte)(data.at<cv::Vec3b>(y, x)(0)) & 0xFF;
            this->data[((height - y - 1) * width + x) * 3 + 1] = (ubyte)(data.at<cv::Vec3b>(y, x)(1)) & 0xFF;
            this->data[((height - y - 1) * width + x) * 3 + 2] = (ubyte)(data.at<cv::Vec3b>(y, x)(2)) & 0xFF;
        }
    }
}


cv::Mat BitmapRender::getData() {
    cv::Mat image(this->height, this->width, CV_8UC3);

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            const uchar r = this->data[((height - y - 1) * width + x) * 3 + 0];
            const uchar g = this->data[((height - y - 1) * width + x) * 3 + 1];
            const uchar b = this->data[((height - y - 1) * width + x) * 3 + 2];

            cv::Vec3b pixel(b, g, r);
            image.at<cv::Vec3b>(y, x) = pixel;
        }
    }

    return image;
}