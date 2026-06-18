#include <GL/glut.h>
#include <cstdlib>

#include "menu.h"
#include "game.h"

enum Screen {
    MENU,
    PLAYING
};

Screen currentScreen = MENU;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (currentScreen == MENU) {
        drawMenu();
    } else if (currentScreen == PLAYING) {
        drawGame();
    }

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if (currentScreen == MENU) {
        if (key == 13) {
            currentScreen = PLAYING;
        }

        if (key == 27) {
            exit(0);
        }
    } else if (currentScreen == PLAYING) {
        gameKeyboard(key, x, y);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 600);
    glutCreateWindow("UFPI Surfers");

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.45f, 0.75f, 1.0f, 1.0f);

    initGame();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, updateGame, 0);

    glutMainLoop();

    return 0;
}