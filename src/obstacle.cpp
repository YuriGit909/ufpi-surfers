#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "model.h"
#include "coin.h"
#include "obstacle.h"
#include "player.h"

using namespace std;

Model busModel("./assets/models/fantasmao.obj");

enum ObstacleType
{
    BUS,
    SPEED_BUMP
};

struct Obstacle
{
    float x;
    float z;
    ObstacleType type;
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

    int type = rand() % 2;

    if (type == 0)
    {
        obs.type = BUS;
    }
    else
    {
        obs.type = SPEED_BUMP;

        // 40% de chance de ter arco de fichas em cima da lombada
        if (rand() % 100 < 40)
        {
            spawnCoinArc(obs.x, obs.z);
        }
    }
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
    for (auto &obs : obstacles)
    {
        if (obs.type == BUS)
        {
            // Ônibus: vermelho, alto e comprido
            glPushMatrix();
            glTranslatef(obs.x, 0.0f, obs.z);
            glScalef(1.3f, 1.3f, 1.3f);
            glRotatef(0.0f, 0.0f, 1.0f, 0.0f);

            busModel.draw();
            glPopMatrix();
        }
        else if (obs.type == SPEED_BUMP)
        {
            // Lombada: laranja, baixa e larga
            glColor3f(1.0f, 0.5f, 0.0f);

            glPushMatrix();
            glTranslatef(obs.x, 0.25f, obs.z);
            glScalef(2.2f, 0.4f, 1.0f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
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

            if (obs.type == SPEED_BUMP)
            {
                if (rand() % 100 < 40)
                {
                    spawnCoinArc(obs.x, obs.z);
                }
            }
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

        if (obs.type == BUS)
{
    if (distanceX < 1.0f &&
        distanceZ < 7.0f &&
        playerY < 2.0f)
    {
        gameOver = true;
        return;
    }
}

        if (obs.type == SPEED_BUMP)
        {
            if (distanceX < 1.0f &&
                distanceZ < 1.0f &&
                playerY < 1.5f)
            {
                gameOver = true;
                return;
            }
        }
    }
}

bool canMoveToLane(float targetX)
{
    for (auto &obs : obstacles)
    {
        float distanceX = fabs(targetX - obs.x);
        float distanceZ = fabs(2.0f - obs.z);

        if (obs.type == BUS &&
            distanceX < 1.0f &&
            distanceZ < 7.0f)
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