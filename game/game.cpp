#include <GL/glut.h>
#include "game.h"
#include "../ui/menu.h"

GameState gameState = MENU; // o jogo começa no menu

void initGame() {

    glEnable(GL_DEPTH_TEST);     // ativa profundidade 3D

    glMatrixMode(GL_PROJECTION); // muda para projeção
    glLoadIdentity();

    gluPerspective(
        60,              // ângulo de visão
        800.0 / 600.0,   // proporção da tela
        0.1,             // distância mínima visível
        500              // distância máxima visível
    );

    glMatrixMode(GL_MODELVIEW); // volta para câmera/modelagem
}

void display() {

    if (gameState == MENU) { // se estiver no menu
        drawMenu();          // desenha tela inicial
        return;              // não desenha o jogo ainda
    }

    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // céu azul
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    gluLookAt(
        0, 5, 10,      // posição da câmera
        0, 1, -20,     // para onde olha
        0, 1, 0        // eixo vertical
    );

    // Por enquanto, tela vazia do jogo
    // Depois vamos chamar aqui:
    // drawRoad();
    // drawUFPI();
    // drawPlayer();
    // drawObstacle();

    glutSwapBuffers();
}

void update(int value) {

    // Atualização do jogo ficará aqui depois
    // Exemplo futuro:
    // if (gameState == PLAYING) {
    //     updatePlayer();
    //     updateObstacle();
    // }

    glutPostRedisplay();              // pede para redesenhar a tela
    glutTimerFunc(16, update, 0);     // chama update novamente em 16ms
}

void keyboard(unsigned char key, int x, int y) {

    if (gameState == MENU && key == 13) { // 13 é ENTER
        gameState = PLAYING;             // começa o jogo
        return;
    }

    if (key == 27) {                      // 27 é ESC
        exit(0);                          // fecha o jogo
    }
}