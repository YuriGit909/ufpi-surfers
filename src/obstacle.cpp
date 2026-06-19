#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "obstacle.h"
#include "player.h"

using namespace std;

struct Obstacle
{
    float x;
    float z;
};

vector<Obstacle> obstacles;

int obstacleCount = 5;
int nextObstacleIncrease = 5000;

extern bool gameOver;
extern bool sideHitWarning;
extern int sideHitTimer;

void createObstacle(float z)
{
    Obstacle obs;

    int lane = rand() % 3;

    if (lane == 0)
        obs.x = -3.0f;
    if (lane == 1)
        obs.x = 0.0f;
    if (lane == 2)
        obs.x = 3.0f;

    obs.z = z;

    obstacles.push_back(obs);
}

void initObstacles()
{
    obstacleCount = 5;
    nextObstacleIncrease = 5000;
    obstacles.clear();
}

void drawObstacles()
{

    glColor3f(1.0f, 0.0f, 0.0f);

    for (auto &obs : obstacles)
    {
        glPushMatrix();
        glTranslatef(obs.x, 0.75f, obs.z);
        glScalef(1.2f, 1.5f, 1.2f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
}

void updateObstacles(float speed, float score)
{
    if (obstacles.empty() && score >= 30)
    {
        for (int i = 0; i < obstacleCount; i++)
        {
            createObstacle(-20.0f - i * 20.0f);
        }
    }

    if (score >= nextObstacleIncrease)
    {
        obstacleCount *= 1.5;
        nextObstacleIncrease *= 2;

        for (int i = 0; i < obstacleCount; i++)
        {
            createObstacle(-20.0f - i * 20.0f);
        }

        printf("Obstaculos: %d | Proximo aumento: %d\n",
               obstacleCount,
               nextObstacleIncrease);
    }

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

bool canMoveToLane(float targetX)
{
    for (auto &obs : obstacles)
    {
        float distanceX = fabs(targetX - obs.x);
        float distanceZ = fabs(2.0f - obs.z);

        if (distanceX < 1.0f && distanceZ < 1.5f)
        {
            if (sideHitWarning)
            {
                gameOver = true;
            }
            else
            {
                sideHitWarning = true;
                sideHitTimer = 0;
            }

            return false;
        }
    }

    return true;
}