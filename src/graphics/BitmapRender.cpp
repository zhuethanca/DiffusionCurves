//
// Created by Ethan on 11/27/2020.
//

#include <GL/glew.h>
#include "graphics/BitmapRender.h"
#include <iostream>

void BitmapRender::render() {
    if (this->data != nullptr)
        glDrawPixels(this->width, this->height, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void BitmapRender::setData(Eigen::SparseMatrix<double> &data, size_t width, size_t height,
             size_t (*index)(size_t, size_t, size_t, size_t)) {
    if (this->data != nullptr) {
        if (this->width * this->height != width * height) {
            delete[] this->data;
            this->data = new ubyte[width * height * 3];
        }
    } else {
        this->data = new ubyte[width * height * 3];
    }
    memset(this->data, 0xFF, sizeof(ubyte) * width * height * 3);
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

void BitmapRender::setData(Eigen::MatrixXd &data, size_t width, size_t height,
                           size_t (*index)(size_t, size_t, size_t, size_t)) {
    if (this->data != nullptr) {
        if (this->width * this->height != width * height) {
            delete[] this->data;
            this->data = new ubyte[width * height * 3];
        }
    } else {
        this->data = new ubyte[width * height * 3];
    }
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

BitmapRender::~BitmapRender() {
    if (this->data != nullptr)
        delete[] this->data;
}
