
#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <vector>
#include "game.h"
#include "player.h"
#include "powerup.h"
#include "ui.h"
#include "coin.h"
#include "obstacle.h"
#include <cstdio>
#include "model.h"

using namespace std;

float cameraX = 0.0f;

bool sideHitWarning = false;
int sideHitTimer = 0;
const int SIDE_HIT_LIMIT = 300; // 5 segundos aprox.

Model* streetModel = nullptr;
Model* streetLowModel = nullptr;

float score = 0;
float pointMultiplier = 0.25;

int frameCounter = 0;
int limiar = 500;

bool gameOver = false;
bool paused = false;

float trackOffset = 0.0f;

float baseSpeed = 0.60f;
float speed = baseSpeed;

void checkPowerUps();

void initGameModels()
{
    streetModel = new Model("./assets/models/street_trees.obj");
    streetLowModel = new Model("./assets/models/street_trees_low.obj");
}

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

    float targetCameraX = getPlayerX() * 0.4f;

    cameraX += (targetCameraX - cameraX) * 0.08f;

    gluLookAt(
        cameraX, 5.0f, 10.0f,
        cameraX, 1.5f, -10.0f,
        0.0f, 1.0f, 0.0f);
}

void initGame()
{

    sideHitWarning = false;
    sideHitTimer = 0;

    speed = baseSpeed;
    pointMultiplier = 0.25;

    limiar = 500;

    score = 0;
    frameCounter = 0;

    gameOver = false;

    trackOffset = 0.0f;

    initObstacles();
    initPowerUps();
    initCoins();
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

/*void drawStreet()
{
    float segmentLength = 150.0f;

    for (int i = 0; i < 3; i++)
    {
        float z = -i * segmentLength + fmod(trackOffset, segmentLength);

        glPushMatrix();
        glTranslatef(0.0f, 0.0f, z);
        streetModel.draw();
        glPopMatrix();
    }
}*/

void drawStreet()
{
    float segmentLength = 150.0f;

    for (int i = 0; i < 3; i++)
    {
        float z = -i * segmentLength + fmod(trackOffset, segmentLength);

        glPushMatrix();
        glTranslatef(0.0f, 0.0f, z);

        
        if (i <= 1)
        {
            if (streetModel != nullptr)
                streetModel->draw();
        }
        else
        {
            if (streetLowModel != nullptr)
                streetLowModel->draw();
        }

        glPopMatrix();
    }
}

void drawGame()
{
    setupGameCamera();

    // drawTrack();
    drawStreet();
    drawObstacles();
    drawPowerUps();
    drawCoins();
    drawPlayer();

    if (isDoublePointsActive())
    {
        glColor3f(1.0f, 0.8f, 0.0f);
        drawText2D(40, 520, "MANGA ATIVA - PONTOS x2");
    }

    if (sideHitWarning && !gameOver)
    {
        glColor3f(1.0f, 0.8f, 0.0f);
        drawText2D(360, 520, "CUIDADO! BATEU DE LADO");
    }
    char scoreText[50];
    sprintf(scoreText, "PONTOS: %d", int(score));

    char coinText[50];
    sprintf(coinText, "FICHAS RU: %d", getRuCoins());

    glColor3f(1.0f, 0.9f, 0.0f);
    drawText2D(40, 490, coinText);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText2D(40, 550, scoreText);
    if (gameOver)
    {

        glColor3f(1.0f, 0.0f, 0.0f);

        drawText2D(430, 330, "GAME OVER");
        drawText2D(350, 290, "PRESSIONE ENTER PARA REINICIAR");
    }

    if (paused)
    {
        glColor3f(1.0f, 1.0f, 0.0f);

        drawText2D(470, 320, "PAUSADO");
        drawText2D(390, 280, "PRESSIONE P PARA CONTINUAR");
    }
}

void updateGame(int value)
{

    if (gameOver)
    {
        glutPostRedisplay();
        glutTimerFunc(16, updateGame, 0);
        return;
    }

    if (paused)
    {
        glutPostRedisplay();
        glutTimerFunc(16, updateGame, 0);
        return;
    }

    trackOffset += speed;

    updateCoins(speed, score);
    checkCoinCollision();
    updatePowerUps(speed);

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

    frameCounter++;

    if (isDoublePointsActive())
        score += pointMultiplier * 2;
    else
        score += pointMultiplier;

    updateObstacles(speed, score);

    updatePlayer();
    if (sideHitWarning)
    {
        sideHitTimer++;

        if (sideHitTimer >= SIDE_HIT_LIMIT)
        {
            sideHitWarning = false;
            sideHitTimer = 0;

            // se tiver implementado cor no player
            // setDamaged(false);
        }
    }
    checkCollision();
    checkPowerUps();
    glutPostRedisplay();
    glutTimerFunc(16, updateGame, 0);
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
    {
        float targetX = getPlayerX() - 7.0f;

        if (canMoveToLane(targetX))
            movePlayerLeft();
    }

    if (key == 'd' || key == 'D')
    {
        float targetX = getPlayerX() + 7.0f;

        if (canMoveToLane(targetX))
            movePlayerRight();
    }

    if (key == 'p' || key == 'P')
    {
        paused = !paused;
        return;
    }

    if (key == 's' || key == 'S')
    {
        roll();
    }

    if (paused)
    {
        return;
    }

    if (key == ' ')
        jump();

    if (key == 27)
        exit(0);
}