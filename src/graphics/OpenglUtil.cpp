//
// Created by Ethan on 11/26/2020.
//

#include "graphics/OpenglUtil.h"
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
