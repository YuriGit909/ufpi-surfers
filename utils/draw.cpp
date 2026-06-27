#include <GL/glut.h>
#include "draw.h"

void drawCube(float x, float y, float z,
              float sx, float sy, float sz) {

    glPushMatrix();              // salva a matriz atual

    glTranslatef(x, y, z);       // move o cubo para a posição desejada
    glScalef(sx, sy, sz);        // altera o tamanho do cubo

    glutSolidCube(1.0);          // desenha um cubo sólido de tamanho 1

    glPopMatrix();               // restaura a matriz anterior
}