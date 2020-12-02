//
// Created by Ethan on 11/23/2020.
//

#include "opengl.h"
// Include standard headers
#include <cstdio>
#include <cstdlib>
//// Include GLEW. Always include it before gl.h and glfw3.h, since it's a bit magic.
//#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
#include <iostream>
#include <graphics/BitmapRender.h>
#include "graphics/Bezier.h"
#include "graphics/ColorCurve.h"
#include "graphics/Util.h"
#include <igl/min_quad_with_fixed.h>
#include "fd_grad.h"

void handleEvents(GLFWwindow* window, int key, int scancode, int action, int mods);

bool handles = true;
bool sBezier = true;
bool sColor = false;
bool sBmap = true;

Bezier bezier(3);
ColorCurve colorCurve(bezier);
BitmapRender bitmapRender;
Eigen::MatrixXd finalImage;
bool rendered = false;

int main() {
    glfwInit();

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Learn OpenGL", nullptr, nullptr);

    int screenWidth, screenHeight;

    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();

        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

//    glewExperimental = GL_TRUE;
//
//    if (GLEW_OK != glewInit())
//    {
//        std::cout << "Failed to initialise GLEW" << std::endl;
//
//        return EXIT_FAILURE;
//    }

    glViewport(0, 0, screenWidth, screenHeight);

    std::cout<<R"(
    H,h      Show/Hide Handles
    B,b      Show/Hide Bezier Curve
    C,c      Show/Hide Color Curves
    E,e      Enter Color
    R,r      Render
    M,m      Show/Hide Bitmap
)";
    int nx = WIDTH;
    int ny = HEIGHT;

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho(0, WIDTH, HEIGHT, 0, -1.0, 1.0);

        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        if (sBmap)
            bitmapRender.render();

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

        glfwSwapBuffers(window);

        glfwPollEvents();

        glfwSetKeyCallback(window, handleEvents);

        if (sBezier && handles)
            bezier.update(window);
        if (sColor && handles)
            colorCurve.update(window);
    }

    glfwTerminate();

    return EXIT_SUCCESS;
}

void handleEvents(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        handles = !(handles);
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        sBezier = !(sBezier);
        if (sBezier) {
            sColor = false;
        }
    }
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        sColor = !(sColor);
        if (sColor) {
            sBezier = false;
        }
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        sBmap = !(sBmap);
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        std::cout << "Processing..." << std::endl;
        Eigen::SparseMatrix<double> rgb;
        colorCurve.renderToMatrix(rgb, WIDTH, HEIGHT);

        Eigen::SparseMatrix<double> norms;
        colorCurve.renderNormToMatrix(norms, WIDTH, HEIGHT);
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

        int nx = WIDTH;
        int ny = HEIGHT;
        Eigen::SparseMatrix<double> G(nx * ny * 2, nx * ny);
        fd_grad(nx, ny, G);

        Eigen::SparseMatrix<double> A = G.transpose() * G;
        Eigen::SparseMatrix<double> Aeq(0, A.rows());

        Eigen::SparseMatrix<double> B = G.transpose() * norms;
        Eigen::MatrixXd Beq = Eigen::MatrixXd::Zero(0, 0);

        igl::min_quad_with_fixed(A, B.toDense(), known, rgbDense, Aeq, Beq, false, finalImage);

        bitmapRender.setData(finalImage, WIDTH, HEIGHT, index);
        rendered = true;
    }
}
