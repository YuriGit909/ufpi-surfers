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

Model bus401("./assets/models/bus_401_.obj");
Model bus365("./assets/models/bus_365.obj");
Model fantasmao("./assets/models/fantasmao.obj");

enum ObstacleType
{
    BUS,
    SPEED_BUMP
};

enum BusType
{
    BUS_401,
    BUS_365,
    BUS_FANTASMAO
};

struct Obstacle
{
    float x;
    float z;

    ObstacleType type;
    BusType busType;

    bool moving;
    float ownSpeed;
};

vector<Obstacle> obstacles;

int obstacleCount = 5;
int nextObstacleIncrease = 5000;

extern bool gameOver;
extern bool sideHitWarning;
extern int sideHitTimer;
float nextRecycleZ = -180.0f;

void createBusAt(float x, float z, bool moving)
{
    Obstacle obs;

    obs.x = x;
    obs.z = z;
    obs.type = BUS;

    obs.moving = moving;
    obs.ownSpeed = moving ? 0.35f : 0.0f;

    int chance = rand() % 100;

    if (chance < 60)
        obs.busType = BUS_401;
    else if (chance < 90)
        obs.busType = BUS_365;
    else
        obs.busType = BUS_FANTASMAO;

    obstacles.push_back(obs);
}

void createObstacleAt(float x, float z, ObstacleType type)
{
    Obstacle obs;
    obs.x = x;
    obs.z = z;
    obs.type = type;
    obs.moving = false;
    obs.ownSpeed = 0.0f;

    if (type == BUS)
    {
        int chance = rand() % 100;

        if (chance < 60)
            obs.busType = BUS_401; // 60%

        else if (chance < 90)
            obs.busType = BUS_365; // 30%

        else
            obs.busType = BUS_FANTASMAO; // 10%

        obs.moving = false;
        obs.ownSpeed = 0.0f;
    }

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
        createBusAt(-7.0f, z, false);
        createBusAt(0.0f, z, false);
    }

    // Padrão 2: ônibus nas laterais, meio livre
    else if (pattern == 1)
    {
        createBusAt(-7.0f, z, false);
        createBusAt(7.0f, z, false);
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
        createBusAt(7.0f, z, rand() % 100 < 35);
    }

    // Padrão 5: obstáculo único
    else
    {
        int lane = rand() % 3;
        float x = lane == 0 ? -7.0f : lane == 1 ? 0.0f
                                                : 7.0f;

        ObstacleType type = rand() % 2 == 0 ? BUS : SPEED_BUMP;
        if (type == BUS)
            createBusAt(x, z, rand() % 100 < 35);
        else
            createObstacleAt(x, z, type);
    }
}
void initObstacles()
{
    obstacleCount = 5;
    nextObstacleIncrease = 5000;
    obstacles.clear();
    nextRecycleZ = -180.0f;
}

void drawObstacles()
{
    for (auto &obs : obstacles)
    {
        if (obs.type == BUS)
        {
            glPushMatrix();

            glTranslatef(obs.x, 0.0f, obs.z);
            glScalef(2.0f, 2.0f, 2.0f);

            switch (obs.busType)
            {
            case BUS_401:
                glScalef(0.07f, 0.07f, 0.07f);
                glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
                bus401.draw();
                break;

            case BUS_365:
                glScalef(1.0f, 1.0f, 1.0f);
                bus365.draw();
                break;

            case BUS_FANTASMAO:
                glScalef(0.7f, 0.7f, 0.7f);
                fantasmao.draw();
                break;
            }

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
    if (obstacles.empty())
    {
        for (int i = 0; i < obstacleCount; i++)
        {
            createObstacle(-120.0f - i * 45.0f);
        }
    }

    if (score >= nextObstacleIncrease)
    {
        obstacleCount = obstacleCount * 1.2f;
        nextObstacleIncrease *= 2;

        obstacles.clear();

        for (int i = 0; i < int(obstacleCount); i++)
        {
            createObstacle(-120.0f - i * 45.0f);
        }

        printf("Obstaculos: %d | Proximo aumento: %d\n",
               obstacleCount,
               nextObstacleIncrease);
    }

    for (auto &obs : obstacles)
    {
        obs.z += speed;

        if (obs.type == BUS && obs.moving)
        {
            obs.z += obs.ownSpeed;
        }

        if (obs.z > 15.0f)
        {
            int lane = rand() % 3;

            if (lane == 0)
                obs.x = -7.0f;
            if (lane == 1)
                obs.x = 0.0f;
            if (lane == 2)
                obs.x = 7.0f;

            obs.z = nextRecycleZ;
            nextRecycleZ -= 50.0f;

            if (nextRecycleZ < -600.0f)
            {
                nextRecycleZ = -180.0f;
            }

            if (obs.type == BUS)
            {
                int chance = rand() % 100;

                if (chance < 60)
                    obs.busType = BUS_401;
                else if (chance < 90)
                    obs.busType = BUS_365;
                else
                    obs.busType = BUS_FANTASMAO;

                obs.moving = (rand() % 100 < 10);
                obs.ownSpeed = obs.moving ? 0.35f : 0.0f;
            }
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
            float hitX = 1.0f;
            float hitZ = 7.0f;

            switch (obs.busType)
            {
            case BUS_401:
                hitX = 1.2f;
                hitZ = 10.0f;
                break;

            case BUS_365:
                hitX = 1.0f;
                hitZ = 7.0f;
                break;

            case BUS_FANTASMAO:
                hitX = 1.0f;
                hitZ = 8.0f;
                break;
            }

            if (distanceX < hitX &&
                distanceZ < hitZ &&
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

        if (obs.type == BUS)
        {
            float hitX = 1.0f;
            float hitZ = 7.0f;

            switch (obs.busType)
            {
            case BUS_401:
                hitX = 1.2f;
                hitZ = 10.0f;
                break;

            case BUS_365:
                hitX = 1.0f;
                hitZ = 7.0f;
                break;

            case BUS_FANTASMAO:
                hitX = 1.0f;
                hitZ = 8.0f;
                break;
            }

            if (distanceX < hitX &&
                distanceZ < hitZ)
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
    }

    return true;
}
