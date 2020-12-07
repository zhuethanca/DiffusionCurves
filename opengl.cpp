//
// Created by Ethan on 11/23/2020.
//

#include "opengl.h"
// Include standard headers
#include <cstdio>
#include <cstdlib>
#include "graphics/GaussianCurve.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <graphics/BitmapRender.h>
#include <graphics/Bezier.h>
#include "graphics/SolveWrappers.h"

#include "diffusion/apply_blur.h"
#include "diffusion/interpolate_control.h"

#include <reconstruction/GaussianStack.h>
#include <reconstruction/EdgeStack.h>
#include <reconstruction/PixelChain.h>
#include <reconstruction/traceEdgePixels.h>
#include <reconstruction/potrace.h>
#include <reconstruction/sampleBezierColours.h>

#include "diffusion/rasterize_color.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iomanip>


int width = WIDTH;
int height = HEIGHT;

void handleEvents(GLFWwindow* window, int key, int scancode, int action, int mods);

bool handles = true;
bool sBezier = true;
bool sColor = false;
bool sGauss = false;
int sBmap = 0;

Bezier bezier(3);
ColorCurve colorCurve(bezier);
GaussianCurve gaussianCurve(bezier);

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

    sBezier = true;

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
            bezier.renderCurve();
            if (handles)
                bezier.renderHandles();
        }

        if (sColor) {
            colorCurve.render();
            if (handles)
                colorCurve.renderHandles();
        }
        if (sGauss) {
            gaussianCurve.render();
            if (handles)
                gaussianCurve.renderHandles();
        }

        glfwSwapBuffers(window);

        glfwPollEvents();

        glfwSetKeyCallback(window, handleEvents);

        if (sBezier && handles) {
            bezier.update(window);
        }

        if (sColor && handles) {
            colorCurve.update(window);
        }

        if (sGauss && handles) {
            gaussianCurve.update(window);
            colorCurve.update(window);
        }
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
            if (rgbRendered)
                bitmapRender.setData(rgbImage, width, height, index);
            break;
        case 2:
            std::cout << "Bitmap Mode: Blur" << std::endl;
            if (blurRendered)
                bitmapRender.setGaussData(blurImage, width, height, index);
            break;
        case 3:
            std::cout << "Bitmap Mode: Final" << std::endl;
            if (finalRendered)
                bitmapRender.setData(finalImage, width, height, index);
            break;
    }
}

double extract(double v);

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
        solve_gaussian(bezier, gaussianCurve, width, height, blurImage);
        blurRendered = true;
        changeBitmap(2);
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        solve_diffusion(bezier, colorCurve, width, height, rgbImage);
        rgbRendered = true;
        changeBitmap(1);
    }

    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        // TODO: Assign a key in the usage printout. J is a placeholder.
        if (!blurRendered || !rgbRendered) {
            std::cout << "Render Color and Blur First!" << std::endl;
        }
        std::cout << "Blurring..." << std::endl;
        apply_blur(rgbImage, blurImage, width, height, finalImage);
        finalRendered = true;
        changeBitmap(3);
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        std::cout << "Handles: " << std::endl;
        for (auto & handle : bezier.handles) {
            std::cout << handle << std::endl;
        }
        std::cout << "P Color: " << std::endl;
        for (auto &ctrl : colorCurve.pControl) {
            std::cout << "(" << ctrl.first << ", " << ctrl.second.asInt << ")" << std::endl;
        }
        std::cout << "N Color: " << std::endl;
        for (auto &ctrl : colorCurve.nControl) {
            std::cout << "(" << ctrl.first << ", " << ctrl.second.asInt << ")" << std::endl;
        }
        std::cout << "Blur: " << std::endl;
        for (auto &ctrl : gaussianCurve.control) {
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

        std::vector<std::vector<Point>> polylines;
        for (int i = 0; i < nChains; i++) {
            PixelChain chain = chains.at(i);

            std::vector<Point> polyline;
            potrace(polyline, chain);
            polylines.emplace_back(polyline);
        }
        bezier.load_polyline(polylines, 3);

        sBezier = true;
        sColor = false;
        sGauss = false;
    }

    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        cv::Mat image = bitmapRender.getData();

        cv::Mat imageLAB;
        cv::cvtColor(image, imageLAB, cv::COLOR_BGR2Lab);

//        for (int c = 0; c < beziers.size(); c++) {
//            sampleBezierColours(*beziers.at(c), *colorCurves.at(c), image, imageLAB, 0.1);
//        }
        sampleBezierColours(bezier, colorCurve, image, imageLAB, 0.1);

        sBezier = false;
        sColor = true;
        sGauss = false;
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        std::cout << "samples=[" << std::endl;
        for (auto & handle : bezier.samples) {
            std::cout << "(" <<
                      (2.0*handle.x/WIDTH-1.0) * 7.5
                      << ", " <<
                      -(2.0*handle.y/HEIGHT-1.0) * 4
                      << ", 0)," << std::endl;
        }
        std::cout << "]" << std::endl << std::endl;
        std::cout << "samples=[" << std::endl;
        for (auto & handle : bezier.samples) {
            std::cout << "(" <<
                      (2.0*handle.x/WIDTH-1.0) * 7.5
                      << ", " <<
                      -(2.0*handle.y/HEIGHT-1.0) * 4
                      << ", 0)," << std::endl;
        }
        std::cout << "]" << std::endl;
        std::cout << "pOffSamples=[" << std::endl;
        for (auto & handle : bezier.pOffset) {
            std::cout << "(" <<
                      (2.0*handle.x/WIDTH-1.0) * 7.5
                      << ", " <<
                      -(2.0*handle.y/HEIGHT-1.0) * 4
                      << ", 0)," << std::endl;
        }
        std::cout << "]" << std::endl << std::endl;
        std::cout << "nOffSamples=[" << std::endl;
        for (auto & handle : bezier.nOffset) {
            std::cout << "(" <<
                      (2.0*handle.x/WIDTH-1.0) * 7.5
                      << ", " <<
                      -(2.0*handle.y/HEIGHT-1.0) * 4
                      << ", 0)," << std::endl;
        }
        std::cout << "]" << std::endl;
        std::cout << "pColors=[" << std::endl;
        std::vector<double> r, g, b;
        interpolate_control(bezier.pOffset, colorCurve.pCurve.controlToIndex(colorCurve.pControl), extractRed, r);
        interpolate_control(bezier.pOffset, colorCurve.pCurve.controlToIndex(colorCurve.pControl), extractGreen, g);
        interpolate_control(bezier.pOffset, colorCurve.pCurve.controlToIndex(colorCurve.pControl), extractBlue, b);
        for (int i = 0; i < r.size(); i ++) {
            ubyte rv = ((ubyte) (r.at(i)*255)) & 0xFF;
            ubyte gv = ((ubyte) (g.at(i)*255)) & 0xFF;
            ubyte bv = ((ubyte) (b.at(i)*255)) & 0xFF;
            int rgb = (rv << 16) | (gv << 8) | bv;
            std::cout << "\"#" << std::setfill('0') << std::setw(6) << std::hex << rgb << "\", " << std::endl;
        }
        std::cout << "]" << std::endl;
        std::cout << "nColors=[" << std::endl;
        r.clear();
        g.clear();
        b.clear();
        interpolate_control(bezier.nOffset, colorCurve.nCurve.controlToIndex(colorCurve.nControl), extractRed, r);
        interpolate_control(bezier.nOffset, colorCurve.nCurve.controlToIndex(colorCurve.nControl), extractGreen, g);
        interpolate_control(bezier.nOffset, colorCurve.nCurve.controlToIndex(colorCurve.nControl), extractBlue, b);
        for (int i = 0; i < r.size(); i ++) {
            ubyte rv = ((ubyte) (r.at(i)*255)) & 0xFF;
            ubyte gv = ((ubyte) (g.at(i)*255)) & 0xFF;
            ubyte bv = ((ubyte) (b.at(i)*255)) & 0xFF;
            int rgb = (rv << 16) | (gv << 8) | bv;
            std::cout << "\"#" << std::setfill('0') << std::setw(6) << std::hex << rgb << "\", " << std::endl;
        }
        std::cout << "]" << std::endl;
        std::cout << "blurColors=[" << std::endl;
        b.clear();
        interpolate_control(bezier.samples, gaussianCurve.curve.controlToIndex(gaussianCurve.control), extract, b);
        double max = 0;
        for (int i = 0; i < b.size(); i ++) {
            max = MAX(max, b.at(i));
        }
        max -= 0.6;
        for (int i = 0; i < b.size(); i ++) {
            ubyte bv = ((ubyte) (((b.at(i)-0.6)/max)*255)) & 0xFF;
            int rgb = (bv << 16) | (bv << 8) | bv;
            std::cout << "\"#" << std::setfill('0') << std::setw(6) << std::hex << rgb << "\", " << std::endl;
        }
        std::cout << "]" << std::endl;
        std::cout << "pHandles={" << std::endl;
        for (auto &handle : colorCurve.pCurve.controlToIndex(colorCurve.pControl)) {
            Point &p = bezier.pOffset.at(handle.first);
            std::cout << "(" <<
                      (2.0*p.x/WIDTH-1.0) * 7.5
                      << ", " <<
                      -(2.0*p.y/HEIGHT-1.0) * 4
                      << "): "
                      << "\"#" << std::setfill('0') << std::setw(6)
                      << std::hex << (handle.second.asInt & 0xFFFFFF) << "\", " << std::endl;
        }
        std::cout << "}" << std::endl;
        std::cout << "nHandles={" << std::endl;
        for (auto &handle : colorCurve.nCurve.controlToIndex(colorCurve.nControl)) {
            Point &p = bezier.nOffset.at(handle.first);
            std::cout << "(" <<
                      (2.0*p.x/WIDTH-1.0) * 7.5
                      << ", " <<
                      -(2.0*p.y/HEIGHT-1.0) * 4
                      << "): "
                      << "\"#" << std::setfill('0') << std::setw(6)
                      << std::hex << (handle.second.asInt & 0xFFFFFF) << "\", " << std::endl;
        }
        std::cout << "}" << std::endl;
        std::cout << "bHandles={" << std::endl;
        for (auto &handle : gaussianCurve.curve.controlToIndex(gaussianCurve.control)) {
            Point &p = bezier.samples.at(handle.first);
            ubyte bv = ((ubyte) (((handle.second-0.6)/max) *255)) & 0xFF;
            int blur = (bv << 16) | (bv << 8) | bv;
            std::cout << "(" <<
                      (2.0*p.x/WIDTH-1.0) * 7.5
                      << ", " <<
                      -(2.0*p.y/HEIGHT-1.0) * 4
                      << "): "
                      << "\"#" << std::setfill('0') << std::setw(6)
                      << std::hex << blur << "\", " << std::endl;
        }
        std::cout << "}" << std::endl;
    }
}
