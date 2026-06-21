#include <GL/glut.h>
#include <cstdlib>

#include "menu.h"
#include "game.h"
#include <ctime>
#include <cstdlib>

enum Screen {
    MENU,
    PLAYING
};

Screen currentScreen = MENU;
bool gameTimerStarted = false;

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
    initGame();

    if (!gameTimerStarted) {
        glutTimerFunc(16, updateGame, 0);
        gameTimerStarted = true;
    }
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
    srand(time(NULL));
   glEnable(GL_DEPTH_TEST);

glEnable(GL_LIGHTING);
glEnable(GL_LIGHT0);
glEnable(GL_COLOR_MATERIAL);
glEnable(GL_NORMALIZE);

glShadeModel(GL_SMOOTH);

GLfloat lightPos[] = {0.0f, 20.0f, 10.0f, 1.0f};
GLfloat ambient[]  = {0.45f, 0.45f, 0.45f, 1.0f};
GLfloat diffuse[]  = {0.9f, 0.9f, 0.9f, 1.0f};
GLfloat specular[] = {0.6f, 0.6f, 0.6f, 1.0f};

glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glClearColor(0.55f, 0.75f, 0.95f, 1.0f);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);


    glutMainLoop();

    return 0;
}