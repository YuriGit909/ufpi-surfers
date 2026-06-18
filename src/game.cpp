#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <vector>
#include "game.h"
#include "player.h"
#include <cstdio>

using namespace std;

struct Obstacle
{
    float x;
    float z;
};

int obstacleCount = 5;
int nextObstacleIncrease = 5000;

float score = 0;
float pointMultiplier = 0.25;

int frameCounter = 0;
int limiar = 500;

vector<Obstacle> obstacles;

bool gameOver = false;

float trackOffset = 0.0f;

float baseSpeed = 0.25f;
float speed = baseSpeed;

void drawCube(float x, float y, float z, float sx, float sy, float sz)
{
    glPushMatrix();

    glTranslatef(x, y, z);
    glScalef(sx, sy, sz);

    glutSolidCube(1.0f);

    glPopMatrix();
}

void setupGameCamera()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0, 1000.0 / 600.0, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
        0.0f, 5.0f, 10.0f,
        0.0f, 1.5f, -10.0f,
        0.0f, 1.0f, 0.0f);
}

void initGame()
{

    obstacleCount = 5;
    nextObstacleIncrease = 5000;

    speed = baseSpeed;
    pointMultiplier = 0.25;

    limiar = 500;

    score = 0;
    frameCounter = 0;

    gameOver = false;

    trackOffset = 0.0f;

    obstacles.clear();
}

void drawTrack()
{
    glColor3f(0.25f, 0.25f, 0.25f);

    for (int i = 0; i < 20; i++)
    {
        float z = -i * 10.0f + fmod(trackOffset, 10.0f);

        drawCube(0.0f, -0.1f, z, 10.0f, 0.2f, 9.5f);
    }

    glColor3f(1.0f, 1.0f, 1.0f);

    for (int i = 0; i < 20; i++)
    {
        float z = -i * 10.0f + fmod(trackOffset, 10.0f);

        drawCube(-1.5f, 0.02f, z, 0.08f, 0.05f, 8.0f);
        drawCube(1.5f, 0.02f, z, 0.08f, 0.05f, 8.0f);
    }
}

void drawObstacles()
{

    glColor3f(1.0f, 0.0f, 0.0f);

    for (auto &obs : obstacles)
    {

        drawCube(
            obs.x,
            0.75f,
            obs.z,
            1.2f,
            1.5f,
            1.2f);
    }
}

void drawGame()
{
    setupGameCamera();

    drawTrack();
    drawObstacles();
    drawPlayer();

    char scoreText[50];
    sprintf(scoreText, "PONTOS: %d", int(score));

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText2D(40, 550, scoreText);
    if (gameOver)
    {

        glColor3f(1.0f, 0.0f, 0.0f);

        drawText2D(430, 330, "GAME OVER");
        drawText2D(350, 290, "PRESSIONE ENTER PARA REINICIAR");
    }
}

void updateGame(int value)
{

    if (score >= limiar)
    {

        pointMultiplier *= 1.15f;
        speed *= 1.15f;
        limiar *= 2;

        printf(
            "Limiar %d | Multiplicador %f | Velocidade %.2f\n",
            limiar,
            pointMultiplier,
            speed);
    }

    if (gameOver)
    {
        glutPostRedisplay();
        glutTimerFunc(16, updateGame, 0);
        return;
    }

    frameCounter++;

    score += pointMultiplier;

    if (obstacles.empty() && score >= 30)
    {
        for (int i = 0; i < obstacleCount; i++)
        {
            Obstacle obs;

            int lane = rand() % 3;

            if (lane == 0)
                obs.x = -3.0f;
            if (lane == 1)
                obs.x = 0.0f;
            if (lane == 2)
                obs.x = 3.0f;

            obs.z = -20.0f - i * 20.0f;

            obstacles.push_back(obs);
        }
    }
    if (score >= nextObstacleIncrease)
    {
        obstacleCount *= 1.15;
        nextObstacleIncrease *= 2;

        obstacles.clear();

        for (int i = 0; i < obstacleCount; i++)
        {
            Obstacle obs;

            int lane = rand() % 3;

            if (lane == 0)
                obs.x = -3.0f;
            if (lane == 1)
                obs.x = 0.0f;
            if (lane == 2)
                obs.x = 3.0f;

            obs.z = -20.0f - i * 20.0f;

            obstacles.push_back(obs);
        }

        printf("Obstaculos: %d | Proximo aumento: %d\n",
               obstacleCount,
               nextObstacleIncrease);
    }

    trackOffset += speed;
    for (auto &obs : obstacles)
    {

        obs.z += speed;

        if (obs.z > 5.0f)
        {

            int lane = rand() % 3;

            if (lane == 0)
                obs.x = -3.0f;
            if (lane == 1)
                obs.x = 0.0f;
            if (lane == 2)
                obs.x = 3.0f;

            obs.z = -120.0f;
        }
    }

    updatePlayer();
    checkCollision();

    glutPostRedisplay();
    glutTimerFunc(16, updateGame, 0);
}

void checkCollision()
{

    float playerX = getPlayerX();
    float playerY = getPlayerY();

    for (auto &obs : obstacles)
    {

        float distanceX = fabs(playerX - obs.x);
        float distanceZ = fabs(2.0f - obs.z);

        if (distanceX < 1.0f &&
            distanceZ < 1.0f &&
            playerY < 2.0f)
        {
            gameOver = true;
        }
    }
}

void drawText2D(float x, float y, const char *text)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1000, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }

    glEnable(GL_DEPTH_TEST);
}

void gameKeyboard(unsigned char key, int x, int y)
{

    if (gameOver)
    {

        if (key == 13)
        { // ENTER
            initGame();
        }

        return;
    }

    if (key == 'a' || key == 'A')
        movePlayerLeft();

    if (key == 'd' || key == 'D')
        movePlayerRight();

    if (key == ' ')
        jump();

    if (key == 27)
        exit(0);
}
