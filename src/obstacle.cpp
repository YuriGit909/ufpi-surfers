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

Model busModel("./assets/models/bus_365.obj");

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

void createObstacleAt(float x, float z, ObstacleType type)
{
    Obstacle obs;
    obs.x = x;
    obs.z = z;
    obs.type = type;

    if (type == SPEED_BUMP && rand() % 100 < 40)
        spawnCoinArc(obs.x, obs.z);

    obstacles.push_back(obs);
}

void createObstacle(float z)
{
    int pattern = rand() % 5;

    // Padrão 1: dois ônibus e uma faixa livre
    if (pattern == 0)
    {
        createObstacleAt(-7.0f, z, BUS);
        createObstacleAt(0.0f, z, BUS);
    }

    // Padrão 2: ônibus nas laterais, meio livre
    else if (pattern == 1)
    {
        createObstacleAt(-7.0f, z, BUS);
        createObstacleAt(7.0f, z, BUS);
    }

    // Padrão 3: três lombadas, uma em cada faixa
    else if (pattern == 2)
    {
        createObstacleAt(-7.0f, z, SPEED_BUMP);
        createObstacleAt(0.0f, z, SPEED_BUMP);
        createObstacleAt(7.0f, z, SPEED_BUMP);
    }

    // Padrão 4: uma lombada e um ônibus
    else if (pattern == 3)
    {
        createObstacleAt(-7.0f, z, SPEED_BUMP);
        createObstacleAt(7.0f, z, BUS);
    }

    // Padrão 5: obstáculo único
    else
    {
        int lane = rand() % 3;
        float x = lane == 0 ? -7.0f : lane == 1 ? 0.0f : 7.0f;

        ObstacleType type = rand() % 2 == 0 ? BUS : SPEED_BUMP;
        createObstacleAt(x, z, type);
    }
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
            glScalef(2.0f, 2.0f, 2.0f);
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
            createObstacle(-200.0f - i * 80.0f);
        }
    }

    if (score >= nextObstacleIncrease)
{
    obstacleCount = obstacleCount * 1.2f;
    nextObstacleIncrease *= 2;

    obstacles.clear();

    for (int i = 0; i < int(obstacleCount); i++)
    {
       createObstacle(-200.0f - i * 80.0f);
    }

    printf("Obstaculos: %d | Proximo aumento: %d\n",
           obstacleCount,
           nextObstacleIncrease);
}

    for (auto &obs : obstacles)
    {
        obs.z += speed;

        if (obs.z > 15.0f)
        {
            int lane = rand() % 3;

            if (lane == 0) obs.x = -7.0f;
            if (lane == 1) obs.x = 0.0f;
            if (lane == 2) obs.x = 7.0f;

            obs.z = -180.0f;

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
