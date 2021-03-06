//
// Created by Ethan on 11/26/2020.
//

#include "util/util.h"
#include <GLFW/glfw3.h>

void drawRect(double x, double y, double width, double height) {
    glVertex2f(x, y);
    glVertex2f( x+width, y);
    glVertex2f( x+width, y+height);
    glVertex2f(x, y+height);
}

void drawCenteredRect(double x, double y, double width, double height) {
    glVertex2f(x-width/2.0, y-height/2.0);
    glVertex2f(x+width/2.0, y-height/2.0);
    glVertex2f(x+width/2.0, y+height/2.0);
    glVertex2f(x-width/2.0, y+height/2.0);
}

size_t index(size_t x, size_t y, size_t width, size_t height) {
    return y*width + x;
}

size_t indexDx(size_t x, size_t y, size_t width, size_t height) {
    return y * width + x;
}

size_t indexDy(size_t x, size_t y, size_t width, size_t height) {
    return (width * height) + (y * width + x);
}
