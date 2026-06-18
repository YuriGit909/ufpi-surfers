#include <GL/glut.h>
#include "menu.h"
#include "camera.h"
#include "model.h"

Model portal("./assets/models/ufpi.obj");

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}



void drawMenu() {
    glClearColor(0.45f, 0.75f, 1.0f, 1.0f);

    setupMenuCamera();

    /* Chão
    glColor3f(0.2f, 0.6f, 0.2f);
    glBegin(GL_QUADS);
        glVertex3f(-20, 0, -20);
        glVertex3f(20, 0, -20);
        glVertex3f(20, 0, 20);
        glVertex3f(-20, 0, 20);
    glEnd(); */

    // Portal
    glPushMatrix();
        glTranslatef(0.0f, -2.0f, -8.0f);
        glScalef(0.35f, 0.35f, 0.35f);
        glRotatef(-92.0f, 0.0f, 1.0f, 0.0f);
        portal.draw();
    glPopMatrix();

    // Texto 2D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1000, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glColor3f(0.0f, 0.0f, 0.8f);
    drawText(430, 500, "UFPI SURFERS");

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(410, 300, "Pressione ENTER para iniciar");
    drawText(445, 260, "Pressione ESC para sair");

    glEnable(GL_DEPTH_TEST);
}