#include <GL/glut.h>
#include "text.h"

void drawText(float x, float y, void* font, const char* text) {

    glMatrixMode(GL_PROJECTION);      // muda para matriz de projeção
    glPushMatrix();                   // salva a projeção 3D atual
    glLoadIdentity();                 // limpa a matriz

    gluOrtho2D(0, 800, 0, 600);       // cria uma tela 2D de 800x600

    glMatrixMode(GL_MODELVIEW);       // muda para matriz de modelagem
    glPushMatrix();                   // salva a câmera atual
    glLoadIdentity();                 // limpa a matriz

    glRasterPos2f(x, y);              // define onde o texto começa

    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(font, text[i]); // desenha letra por letra
    }

    glPopMatrix();                    // restaura a câmera 3D

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();                    // restaura a projeção 3D

    glMatrixMode(GL_MODELVIEW);       // volta para o modo normal
}