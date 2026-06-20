#include <GL/glut.h>
#include <cmath>
#include "menu.h"
#include "camera.h"
#include "model.h"

Model portal("./assets/models/ufpi.obj");

void drawStrokeText(float x, float y, float scale, const char* text)
{
    glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(scale, scale, scale);

        for (int i = 0; text[i] != '\0'; i++)
        {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);
        }
    glPopMatrix();
}

void drawBitmapText(float x, float y, const char* text)
{
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

void drawMenuBox(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);
        glVertex2f(x2, y2);
        glVertex2f(x1, y2);
    glEnd();
}

void drawMenu()
{
    glClearColor(0.45f, 0.75f, 1.0f, 1.0f);

    setupMenuCamera();

    // Portal
    glPushMatrix();
        glTranslatef(0.0f, -2.0f, -8.0f);
        glScalef(0.35f, 0.35f, 0.35f);
        glRotatef(-92.0f, 0.0f, 1.0f, 0.0f);
        portal.draw();
    glPopMatrix();

    // Interface 2D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1000, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    // Sombra do título
    glColor3f(0.0f, 0.0f, 0.0f);
    drawStrokeText(248, 503, 0.45f, "UFPI SURFERS");

    // Título principal
    glColor3f(0.0f, 0.2f, 0.8f);
    drawStrokeText(245, 506, 0.45f, "UFPI SURFERS");

    // Caixa do botão iniciar
    glColor3f(0.0f, 0.15f, 0.45f);
    drawMenuBox(345, 185, 655, 245);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawBitmapText(405, 207, "PRESSIONE ENTER");

    // Texto sair
    glColor3f(0.95f, 0.95f, 0.95f);
    drawBitmapText(425, 155, "ESC PARA SAIR");

    glEnable(GL_DEPTH_TEST);
}