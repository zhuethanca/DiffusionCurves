//
// Created by Ethan on 11/23/2020.
//

#include "opengl.h"
// Include standard headers
#include <cstdio>
#include <cstdlib>
#include "graphics/GaussianCurve.h"
#include <unsupported/Eigen/MatrixFunctions>
#include <GLFW/glfw3.h>
#include <iostream>
#include <graphics/BitmapRender.h>
#include <graphics/Bezier.h>
#include "graphics/ColorCurve.h"
#include "graphics/Util.h"
#include <igl/min_quad_with_fixed.h>
#include <iomanip>
#include <unordered_map>
#include "fd_grad.h"

#include <reconstruction/GaussianStack.h>
#include <reconstruction/EdgeStack.h>
#include <reconstruction/PixelChain.h>
#include <reconstruction/traceEdgePixels.h>
#include <reconstruction/potrace.h>
#include <reconstruction/sampleBezierColours.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


int width = WIDTH;
int height = HEIGHT;

void handleEvents(GLFWwindow* window, int key, int scancode, int action, int mods);

bool handles = true;
bool sBezier = true;
bool sColor = false;
bool sGauss = false;
int sBmap = 0;

Bezier *beziers = new Bezier(3);
ColorCurve *colorCurves = new ColorCurve(beziers[0]);
GaussianCurve *gaussianCurves = new GaussianCurve(beziers[0]);

BitmapRender bitmapRender;
Eigen::MatrixXd rgbImage;
Eigen::MatrixXd blurImage;
Eigen::MatrixXd finalImage;
cv::Mat backgroundImage;

bool rgbRendered = false;
bool blurRendered = false;
bool finalRendered = false;
bool imageRendered = false;


int main(int argc, char** argv) {
    if (argc >= 2) {
        char* filepath = argv[1];
        backgroundImage = cv::imread(argv[1]);

        width = backgroundImage.cols;
        height = backgroundImage.rows;

        bitmapRender.setData(backgroundImage);
        imageRendered = true;
        sBmap = 4;
    }

    glfwInit();

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(width, height, "Diffusion Curves", nullptr, nullptr);

    int screenWidth, screenHeight;

    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glViewport(0, 0, screenWidth, screenHeight);

    std::cout<<R"(
    H,h      Show/Hide Handles
    B,b      Show/Hide Bezier Curve
    C,c      Show/Hide Color Curves
    G,g      Show/Hide Gaussian Blur Curves
    J,j      Render Gaussian Blur Map
    E,e      Enter Color/Sigma
    R,r      Render

    M,m      Change Bitmap Mode

    A,a      Auto-generate Curves from Image
)";

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.5, 0.5, 0.5, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1.0, 1.0);

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        if (sBmap) {
            if (sBmap == 1 && rgbRendered)
                bitmapRender.render();
            if (sBmap == 2 && blurRendered)
                bitmapRender.render();
            if (sBmap == 3 && finalRendered)
                bitmapRender.render();
            if (sBmap == 4 && imageRendered)
                bitmapRender.render();
        }

        if (sBezier) {
            beziers[0].renderCurve();
            if (handles)
                beziers[0].renderHandles();
        }

        if (sColor) {
            colorCurves[0].render();
            if (handles)
                colorCurves[0].renderHandles();
        }
        if (sGauss) {
            gaussianCurves[0].render();
            if (handles)
                gaussianCurves[0].renderHandles();
        }

        glfwSwapBuffers(window);

        glfwPollEvents();

        glfwSetKeyCallback(window, handleEvents);

        if (sBezier && handles)
            beziers[0].update(window);
        if (sColor && handles)
            colorCurves[0].update(window);
        if (sGauss && handles)
            gaussianCurves[0].update(window);
            colorCurves[0].update(window);
    }

    glfwTerminate();

    return EXIT_SUCCESS;
}

void changeBitmap(int to) {
    if (to == -1)
        sBmap = (sBmap+1)%4;
    else
        sBmap = to % 4;
    switch (sBmap) {
        case 0:
            std::cout << "Bitmap Mode: Off" << std::endl;
            break;
        case 1:
            std::cout << "Bitmap Mode: Color" << std::endl;
            bitmapRender.setData(rgbImage, width, height, index);
            break;
        case 2:
            std::cout << "Bitmap Mode: Blur" << std::endl;
            bitmapRender.setGaussData(blurImage, width, height, index);
            break;
        case 3:
            std::cout << "Bitmap Mode: Final" << std::endl;
            bitmapRender.setData(finalImage, width, height, index);
            break;
    }
}

void generateGaussian(Eigen::MatrixXd &kernel, int x, int y, int width, int height,
                      double sigma, int *kx, int *ky) {
    int size = ceil(3*sigma);
    int ls = MIN(size, x);
    int rs = MIN(size, width-x-1);
    int ts = MIN(size, y);
    int bs = MIN(size, height-y-1);
    kernel.resize(ls+rs+1, ts+bs+1);
    *kx = x-ls;
    *ky = y-ts;
    for (int rx = x-ls; rx <= x+rs; rx ++) {
        for (int ry = y-ts; ry <= y + bs; ry ++) {
            kernel(rx-*kx, ry-*ky) = (rx-x)*(rx-x) + (ry-y)*(ry-y);
        }
    }
    kernel /= -2*sigma*sigma;
    kernel.array() = Eigen::exp(kernel.array());
    kernel /= (2*EIGEN_PI*sigma*sigma);
}

void handleEvents(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        handles = !(handles);
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        sBezier = !(sBezier);
        if (sBezier) {
            sColor = false;
            sGauss = false;
        }
    }
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        sColor = !(sColor);
        if (sColor) {
            sBezier = false;
            sGauss = false;
        }
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        sGauss = !(sGauss);
        if (sGauss) {
            sBezier = false;
            sColor = false;
        }
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        changeBitmap(-1);
    }
    if (key == GLFW_KEY_J && action == GLFW_PRESS) {
        std::cout << "Processing..." << std::endl;
        Eigen::SparseMatrix<double> blur;
        gaussianCurves[0].renderToMatrix(blur, width, height);
        std::set<int> known_set;
        for (Eigen::SparseMatrix<double>::InnerIterator it(blur, 0); it; ++it) {
            known_set.emplace(it.row());
        }
        Eigen::MatrixXd blurDense(known_set.size(), 1);
        Eigen::VectorXi known(known_set.size());
        {
            int i = 0;
            for (auto k : known_set) {
                known(i) = k;
                blurDense(i, 0) = MAX(blur.coeff(k, 0), 0.6); //Cutoff due to numerical instability
                i++;
            }
        }

        int nx = width;
        int ny = height;
        Eigen::SparseMatrix<double> G(nx * ny * 2, nx * ny);
        fd_grad(nx, ny, G);

        Eigen::SparseMatrix<double> A = G.transpose() * G;
        Eigen::SparseMatrix<double> Aeq(0, A.rows());

        Eigen::SparseMatrix<double> B(G.rows(), 1);
        Eigen::MatrixXd Beq = Eigen::MatrixXd::Zero(0, 0);

        igl::min_quad_with_fixed(A, B.toDense(), known, blurDense, Aeq, Beq, false, blurImage);
        changeBitmap(2);
        blurRendered = true;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        std::cout << "Processing..." << std::endl;
        Eigen::SparseMatrix<double> rgb;

        Eigen::SparseMatrix<double> norms;
        colorCurves[0].renderNormToMatrix(norms, width, height);
//        for (int y = 100; y <= 200; y ++) {
//            std::cout << "\t";
//            for (int x = 97; x <= 106; x ++) {
//                std::cout << norms.coeff(indexDx(x, y, WIDTH, HEIGHT), 0) << "\t";
//            }
//            std::cout << std::endl;
//        }
        std::cout << "Solving..." << std::endl;

        std::set<int> known_set;
        for (int k = 0; k < 3; k++) {
            for (Eigen::SparseMatrix<double>::InnerIterator it(rgb, k); it; ++it) {
                known_set.emplace(it.row());
            }
        }
        Eigen::MatrixX3d rgbDense(known_set.size(), 3);
        Eigen::VectorXi known(known_set.size());
        {
            int i = 0;
            for (auto k : known_set) {
                known(i) = k;
                rgbDense(i, 0) = rgb.coeff(k, 0);
                rgbDense(i, 1) = rgb.coeff(k, 1);
                rgbDense(i, 2) = rgb.coeff(k, 2);
                i++;
            }
        }

        int nx = width;
        int ny = height;
        Eigen::SparseMatrix<double> G(nx * ny * 2, nx * ny);
        fd_grad(nx, ny, G);
        G *= 1.5;

        Eigen::SparseMatrix<double> A = G.transpose() * G;
        Eigen::SparseMatrix<double> Aeq(0, A.rows());

        Eigen::SparseMatrix<double> B = G.transpose() * norms;
        Eigen::MatrixXd Beq = Eigen::MatrixXd::Zero(0, 0);

        igl::min_quad_with_fixed(A, B.toDense(), known, rgbDense, Aeq, Beq, false, rgbImage);

        changeBitmap(1);
        rgbRendered = true;
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        // TODO: Assign a key in the usage printout. J is a placeholder.
        if (!blurRendered || !rgbRendered) {
            std::cout << "Render Color and Blur First!" << std::endl;
        }
        std::cout << "Blurring..." << std::endl;
        finalImage.resizeLike(rgbImage);
        std::unordered_map<double, Eigen::SparseMatrix<double, Eigen::RowMajor>> kernels;
        std::vector<Tripletd> matList(ceil(blurImage.maxCoeff()*6)*rgbImage.rows());

        for (int x = 0; x < width; x ++) {
            std::cout << "\r" << std::setprecision(4) << ((x)/(double) width) * 100.0 << "%         " << std::flush;
            for (int y = 0; y < HEIGHT; y ++) {
                uint32_t idx = index(x, y, width, height);
                double sigma = blurImage(idx, 0);
                Eigen::MatrixXd kernel;
                int kx, ky;
                generateGaussian(kernel, x, y, width, height, sigma, &kx, &ky);
                for (int rx = 0; rx < kernel.rows(); rx ++) {
                    for (int ry = 0; ry < kernel.cols(); ry ++) {
                        matList.emplace_back(index(x, y, width, height), index(kx+rx, ky+ry, width, height), kernel(rx, ry));
                    }
                }
            }
        }
        std::cout << std::endl;
        Eigen::SparseMatrix<double, Eigen::RowMajor> kernelMatrix(rgbImage.rows(), rgbImage.rows());
        kernelMatrix.setFromTriplets(matList.begin(), matList.end());
        finalImage = kernelMatrix*rgbImage;
        finalRendered = true;
        changeBitmap(3);
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        std::cout << "Handles: " << std::endl;
        for (auto & handle : beziers->handles) {
            std::cout << handle << std::endl;
        }
        std::cout << "P Color: " << std::endl;
        for (auto &ctrl : colorCurves->pControl) {
            std::cout << "(" << ctrl.first << ", " << ctrl.second.asInt << ")" << std::endl;
        }
        std::cout << "N Color: " << std::endl;
        for (auto &ctrl : colorCurves->nControl) {
            std::cout << "(" << ctrl.first << ", " << ctrl.second.asInt << ")" << std::endl;
        }
        std::cout << "Blur: " << std::endl;
        for (auto &ctrl : gaussianCurves->control) {
            std::cout << "(" << ctrl.first << ", " << ctrl.second << ")" << std::endl;
        }
    }

    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        cv::Mat image = bitmapRender.getData();

        GaussianStack stack(image);
        EdgeStack edges(stack, 80, 200);

        std::vector<PixelChain> chains;
        traceEdgePixels(chains, edges.layer(0), 5);

        const int nChains = chains.size();

        for (int i = 0; i < nChains; i++) {
            PixelChain chain = chains.at(i);

            std::vector<Point> polyline;
            potrace(polyline, chain);

            // Set the Bezier curve from this polyline.
            beziers = new Bezier(polyline, 3);
            colorCurves = new ColorCurve(beziers[0]);
            gaussianCurves = new GaussianCurve(beziers[0]);
        }

        sBezier = true;
        sColor = false;
        sGauss = false;
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        cv::Mat image = bitmapRender.getData();

        cv::Mat imageLAB;
        cv::cvtColor(image, imageLAB, cv::COLOR_BGR2Lab);

        sampleBezierColours(beziers[0], colorCurves[0], image, imageLAB, 0.1);

        sBezier = false;
        sColor = true;
        sGauss = false;
    }
}
