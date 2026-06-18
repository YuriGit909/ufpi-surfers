#include <GL/glut.h>
#include "menu.h"
#include "text.h"
#include "../utils/draw.h"

void drawMenu() {

    glClearColor(0.08f, 0.20f, 0.38f, 1.0f);   // cor azul escura do fundo
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();                          // limpa a câmera

    gluLookAt(
        0, 5, 12,      // posição da câmera
        0, 1, -20,     // para onde a câmera olha
        0, 1, 0        // eixo vertical
    );

    // Pista de fundo
    glColor3f(0.25f, 0.25f, 0.25f);
    drawCube(0, -0.1f, -20, 10, 0.2f, 80);

    // Grama esquerda
    glColor3f(0.1f, 0.6f, 0.1f);
    drawCube(-8, -0.15f, -20, 6, 0.2f, 80);

    // Grama direita
    drawCube(8, -0.15f, -20, 6, 0.2f, 80);

    // Título
    glColor3f(1.0f, 0.85f, 0.15f);
    drawText(275, 400, GLUT_BITMAP_TIMES_ROMAN_24, "UFPI SURFERS");

    // Subtítulo
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(210, 355, GLUT_BITMAP_HELVETICA_18, "Corra pelo campus e desvie dos obstaculos");

    // Instruções
    glColor3f(0.8f, 0.95f, 1.0f);
    drawText(290, 290, GLUT_BITMAP_HELVETICA_18, "A / D  - mover");
    drawText(290, 260, GLUT_BITMAP_HELVETICA_18, "ESPACO - pular");
    drawText(290, 230, GLUT_BITMAP_HELVETICA_18, "R      - reiniciar");

    // Texto piscando
    int tempo = glutGet(GLUT_ELAPSED_TIME);    // pega tempo desde que abriu

    if ((tempo / 500) % 2 == 0) {              // faz piscar a cada meio segundo
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(260, 170, GLUT_BITMAP_HELVETICA_18, "PRESSIONE ENTER PARA INICIAR");
    }

    glutSwapBuffers();                         // mostra tudo na tela
}