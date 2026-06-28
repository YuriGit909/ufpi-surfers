#ifndef MENU_H
#define MENU_H

#include <GL/glut.h>

void drawMenu();
void drawMenuIntro(float cameraZ);
void initMenu();

void drawSkyGradient();
void drawSunRays();
void drawCloud(float x, float y, float w, float h);

GLuint loadMenuTexture(const char *file);

#endif