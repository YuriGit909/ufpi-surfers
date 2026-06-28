#include <GL/glew.h>
#include <GL/glut.h>


#include "audio.h"
#include <cstdlib>

#include "menu.h"
#include "game.h"
#include "powerup.h"
#include "player.h"
#include "obstacle.h"
#include <ctime>
#include <cstdlib>

enum Screen {
    MENU,
    INTRO,
    PLAYING
};

Screen currentScreen = MENU;
float fadeAlpha = 0.0f;
bool fadingIntro = false;

float introCameraZ = 18.0f;
bool gameTimerStarted = false;

void drawFade()
{
    if (fadeAlpha <= 0.0f)
        return;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1000, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);

    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(1000, 0);
        glVertex2f(1000, 600);
        glVertex2f(0, 600);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (currentScreen == MENU) {
        drawMenu();
    } else if (currentScreen == INTRO) {
        drawMenuIntro(introCameraZ);
        drawFade();
    } else if (currentScreen == PLAYING) {
        drawGame();
    }

    glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y) {
    if (currentScreen == MENU) {
        if (key == 13) {
            currentScreen = INTRO;
            introCameraZ = 18.0f;
            fadeAlpha = 0.0f;
            fadingIntro = false;
        }

        if (key == 27) {
            exit(0);
        }

        return;
    }

    if (currentScreen == PLAYING) {
        gameKeyboard(key, x, y);
    }
}

void updateIntro(int value)
{
    if (currentScreen == INTRO)
    {
        introCameraZ -= 0.15f;

        if (introCameraZ <= -6.0f)
        {
            fadingIntro = true;
        }

        if (fadingIntro)
        {
            fadeAlpha += 0.03f;

            if (fadeAlpha >= 1.0f)
            {
                fadeAlpha = 1.0f;
                currentScreen = PLAYING;

                initGame();

                if (!gameTimerStarted)
                {
                    glutTimerFunc(16, updateGame, 0);
                    gameTimerStarted = true;
                }
            }
        }

        glutPostRedisplay();
    }

    glutTimerFunc(16, updateIntro, 0);
}

int main(int argc, char** argv) {
  
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 600);
    glutCreateWindow("UFPI Surfers");
    glewInit();
    initMenu();
    initAudio();
    playMusic();

    atexit(stopAudio);
      initPlayerModel();
    initObstacleModels();
    initPowerUpModels();
    initGameModels();
    srand(time(NULL));
   glEnable(GL_DEPTH_TEST);

glEnable(GL_LIGHTING);
glEnable(GL_LIGHT0);
glEnable(GL_COLOR_MATERIAL);
glEnable(GL_NORMALIZE);
glEnable(GL_TEXTURE_2D);

glTexEnvi(
    GL_TEXTURE_ENV,
    GL_TEXTURE_ENV_MODE,
    GL_MODULATE
);

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

    glClearColor(0.05f, 0.25f, 0.75f, 1.0f);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, updateIntro, 0);


    glutMainLoop();

    return 0;
}