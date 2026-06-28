#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "model.h"
#include "coin.h"
#include "obstacle.h"
#include "player.h"
#include "powerup.h"

using namespace std;

Model *speedBumpModel = nullptr;
Model bus401("./assets/models/bus_401_.obj");
Model bus365("./assets/models/bus_365.obj");
Model fantasmao("./assets/models/fantasmao.obj");
Model *lowBarrierModel = nullptr;

void initObstacleModels()
{
    speedBumpModel = new Model("./assets/models/speedbump.obj");
    lowBarrierModel = new Model("./assets/models/barrier.glb");
}

enum ObstacleType
{
    BUS,
    SPEED_BUMP,
    LOW_BARRIER
};
enum BusType
{
    BUS_401,
    BUS_365,
    BUS_FANTASMAO
};

struct Obstacle
{
    float x, z;
    ObstacleType type;
    BusType busType;
    bool moving;
    float ownSpeed;
};

vector<Obstacle> obstacles;

int obstacleCount = 5;
int nextObstacleIncrease = 5000;
float nextRecycleZ = -180.0f;

extern bool gameOver;
extern bool sideHitWarning;
extern int sideHitTimer;

struct HitBox
{
    float hitX, hitZ, hitH;
};

static HitBox getBusHitBox(BusType t)
{
    switch (t)
    {
    case BUS_401:
        return {1.2f, 10.0f, 4.0f};
    case BUS_365:
        return {1.0f, 7.0f, 3.5f};
    case BUS_FANTASMAO:
        return {1.0f, 8.0f, 3.5f};
    }
    return {1.0f, 7.0f, 3.5f};
}

static float playerTopY() { return getPlayerY() + 0.5f; }
static float playerBottomY() { return getPlayerY() - 0.5f; }

static float playerEffectiveTopY()
{
    if (isRolling())
        return 0.3f + 0.25f; // 0.55
    return playerTopY();
}

// -----------------------------------------------------------------
BusType randomBusType()
{
    int r = rand() % 100;
    if (r < 60)
        return BUS_401;
    if (r < 90)
        return BUS_365;
    return BUS_FANTASMAO;
}

void createBusAt(float x, float z, bool moving)
{
    Obstacle obs;
    obs.x = x;
    obs.z = z;
    obs.type = BUS;
    obs.busType = randomBusType();
    obs.moving = moving;
    obs.ownSpeed = moving ? 0.35f : 0.0f;
    obstacles.push_back(obs);
}

void clearNearbyObstacles(float z)
{
    for (auto &obs : obstacles)
    {
        if (fabs(obs.z - z) < 8.0f)
            obs.z = 100.0f;
    }
}

void createObstacleAt(float x, float z, ObstacleType type)
{
    Obstacle obs;
    obs.x        = x;
    obs.z        = z;
    obs.type     = type;
    obs.moving   = false;
    obs.ownSpeed = 0.0f;

    if (type == BUS)
        obs.busType = randomBusType();

    if (type == SPEED_BUMP && rand() % 100 < 40)
        spawnCoinArc(obs.x, obs.z);

    // Moedas rasteiras embaixo da barreira
    if (type == LOW_BARRIER && rand() % 100 < 70)
        spawnCoinArcLow(obs.x, obs.z);

    obstacles.push_back(obs);
}

// espaçamento mínimo entre grupos de obstáculos — aumenta conforme
// a velocidade para evitar sobreposição
static float obstacleSpacing(float speed)
{
    // base 80 unidades; garante ao menos 3 segundos de distância
    float minGap = speed * 60.0f * 3.0f; // 3 segundos a 60fps
    return fmaxf(80.0f, minGap);
}

bool hasObstacleNear(float x, float z, float radiusX, float radiusZ)
{
    for (auto& obs : obstacles)
    {
        if (obs.type == BUS)
        {
            if (fabs(x - obs.x) < radiusX && fabs(z - obs.z) < radiusZ)
                return true;
        }
    }
    return false;
}

void createObstacle(float z)
{
    // 50% ônibus, 50% lombada
    int obstacleKind = rand() % 3;

    int lane = rand() % 3;
    float laneX[3] = {-7.0f, 0.0f, 7.0f};

    if (obstacleKind == 0)
    {
        int pattern = rand() % 3;

        // Padrão 1: um ônibus numa faixa aleatória
        if (pattern == 0)
        {
            createBusAt(laneX[lane], z, rand() % 100 < 35);
        }
        // Padrão 2: dois ônibus, uma faixa livre
        else if (pattern == 1)
        {
            int free = rand() % 3;
            for (int i = 0; i < 3; i++)
                if (i != free)
                    createBusAt(laneX[i], z, false);
        }
        // Padrão 3: ônibus nas laterais
        else
        {
            createBusAt(-7.0f, z, rand() % 100 < 35);
            createBusAt(7.0f, z, rand() % 100 < 35);
        }
    }
    else if (obstacleKind == 1)
    {
        int pattern = rand() % 3;

        // Padrão 1: uma lombada numa faixa aleatória
        if (pattern == 0)
        {
            createObstacleAt(laneX[lane], z, SPEED_BUMP);
        }
        // Padrão 2: duas lombadas, uma faixa livre
        else if (pattern == 1)
        {
            int free = rand() % 3;
            for (int i = 0; i < 3; i++)
                if (i != free)
                    createObstacleAt(laneX[i], z, SPEED_BUMP);
        }
        // Padrão 3: três lombadas
        else
        {
            for (int i = 0; i < 3; i++)
                createObstacleAt(laneX[i], z, SPEED_BUMP);
        }
    }
    else
    {
        int pattern = rand() % 3;

        if (pattern == 0)
        {
            createObstacleAt(laneX[lane], z, LOW_BARRIER);
        }

        else if (pattern == 1)
        {
            int free = rand() % 3;
            for (int i = 0; i < 3; i++)
                if (i != free)
                    createObstacleAt(laneX[i], z, LOW_BARRIER);
        }
        else
        {
            for (int i = 0; i < 3; i++)
                createObstacleAt(laneX[i], z, LOW_BARRIER);
        }
    }
}

void initObstacles()
{
    obstacleCount = 5;
    nextObstacleIncrease = 5000;
    nextRecycleZ = -180.0f;
    obstacles.clear();
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
            if (speedBumpModel)
            {
                glPushMatrix();
                glTranslatef(obs.x, 0.0f, obs.z);
                glScalef(1.5f, 6.0f, 3.0f);
                speedBumpModel->draw();
                glPopMatrix();
            }
        }
        else if (obs.type == LOW_BARRIER)
        {
            if (lowBarrierModel)
            {
                glPushMatrix();
                glTranslatef(obs.x, 1.8f, obs.z);
                glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
                glScalef(1.0f, 1.0f, 1.0f);

                lowBarrierModel->draw();

                glPopMatrix();
            }
        }
    }
}

void updateObstacles(float speed, float score)
{
    float spacing = obstacleSpacing(speed);

    if (obstacles.empty())
    {
        for (int i = 0; i < obstacleCount; i++)
            createObstacle(-200.0f - i * spacing);
    }

    if (score >= nextObstacleIncrease)
    {
        obstacleCount = (int)(obstacleCount * 1.2f);
        nextObstacleIncrease *= 2;

        obstacles.erase(
            remove_if(obstacles.begin(), obstacles.end(),
                      [](const Obstacle &o) { return o.z > 15.0f; }),
            obstacles.end());

        float farthestZ = -200.0f;
        for (auto &obs : obstacles)
            farthestZ = fminf(farthestZ, obs.z);

        for (int i = 0; i < obstacleCount; i++)
            createObstacle(farthestZ - (i + 1) * spacing);

        nextRecycleZ = farthestZ - obstacleCount * spacing;

        printf("Obstaculos: %d | Proximo aumento: %d\n",
               obstacleCount, nextObstacleIncrease);
    }

    // ← esse bloco estava faltando
    for (auto &obs : obstacles)
    {
        obs.z += speed;

        if (obs.type == BUS && obs.moving)
            obs.z += obs.ownSpeed;

        if (obs.z > 15.0f)
        {
            int lane = rand() % 3;
            obs.x = lane == 0 ? -7.0f : lane == 1 ? 0.0f : 7.0f;
            obs.z = nextRecycleZ;
            nextRecycleZ -= spacing;

            if (nextRecycleZ < -600.0f)
                nextRecycleZ = -spacing * obstacleCount;

            int newKind = rand() % 3;
            if (newKind == 0)
            {
                obs.type     = BUS;
                obs.busType  = randomBusType();
                obs.moving   = (rand() % 100 < 10);
                obs.ownSpeed = obs.moving ? 0.35f : 0.0f;
            }
            else if (newKind == 1)
            {
                obs.type = SPEED_BUMP;
                if (rand() % 100 < 40)
                    spawnCoinArc(obs.x, obs.z);
            }
            else
            {
                obs.type = LOW_BARRIER;
                if (rand() % 100 < 70)
                    spawnCoinArcLow(obs.x, obs.z);
            }
        }
    }
}

void checkCollision()
{
    float px = getPlayerX();
    float ptY = playerEffectiveTopY();
    float pbY = isRolling() ? 0.05f : playerBottomY();

    for (auto &obs : obstacles)
    {
        float distX = fabs(px - obs.x);
        float distZ = fabs(2.0f - obs.z);

        if (obs.type == BUS)
        {
            HitBox hb = getBusHitBox(obs.busType);

            if (distX < hb.hitX && distZ < hb.hitZ && ptY < hb.hitH)
            {
                if (isFinalExamActive())
                {
                    consumeFinalExam();
                    clearNearbyObstacles(obs.z);
                    return;
                }

                gameOver = true;
                return;
            }
        }

        if (obs.type == SPEED_BUMP)
        {
            const float bumpHalfX = 1.5f;
            const float bumpHalfZ = 3.0f;
            const float bumpH = 0.6f;

            if (distX < bumpHalfX && distZ < bumpHalfZ && pbY < bumpH)
            {
                if (isFinalExamActive())
                {   
                    consumeFinalExam();
                    clearNearbyObstacles(obs.z);
                    return;
                }

                gameOver = true;
                return;
            }
        }

        if (obs.type == LOW_BARRIER)
        {
            const float barrierHalfX = 2.0f;
            const float barrierHalfZ = 1.2f;
            const float barrierTopY = 1.8f;

            if (distX < barrierHalfX && distZ < barrierHalfZ)
            {
                bool rolledUnder = isRolling();
                bool jumpedOver = playerBottomY() >= barrierTopY;

                if (!rolledUnder && !jumpedOver)
                {
                    if (isFinalExamActive())
                    {
                        consumeFinalExam();
                        clearNearbyObstacles(obs.z);
                        return;
                    }

                    gameOver = true;
                    return;
                }
            }
        }
    }
}

bool canMoveToLane(float targetX)
{
    float ptY = playerEffectiveTopY();

    for (auto &obs : obstacles)
    {
        float distX = fabs(targetX - obs.x);
        float distZ = fabs(2.0f - obs.z);

        if (obs.type == BUS)
        {
            HitBox hb = getBusHitBox(obs.busType);

            if (distX < hb.hitX && distZ < hb.hitZ && ptY < hb.hitH)
            {
                if (sideHitWarning)
                    gameOver = true;
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