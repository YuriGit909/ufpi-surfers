#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

#include "professor.h"
#include "model.h"
#include "coin.h"
#include "obstacle.h"
#include "player.h"
#include "powerup.h"
#include "game.h"

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

enum ChaserState
{
    CHASER_IDLE,
    CHASER_TRACKING,
    CHASER_SWITCHING,
    CHASER_RETREATING
};

struct Obstacle
{
    float x, z;
    ObstacleType type;
    BusType busType;
    bool moving;
    float ownSpeed;

    bool isChaser;
    ChaserState chaserState;
    float chaserTargetX;
    float chaserOriginX;
    int chaserCooldown;
};

vector<Obstacle> obstacles;

int obstacleCount = 5;
int nextObstacleIncrease = 5000;
float nextRecycleZ = -180.0f;

extern bool gameOver;
extern bool sideHitWarning;
extern int sideHitTimer;

struct PlayerProfile
{
    int jumpCount;
    int rollCount;
    int laneChanges;
    int sampleFrames;

    float lastLaneX;
    int consecutiveSameLane;
};

static PlayerProfile gProfile = {0, 0, 0, 0, 0.0f, 0};

static void updateProfile()
{
    gProfile.sampleFrames++;

    float px = getPlayerX();

    if (fabs(px - gProfile.lastLaneX) > 3.5f)
    {
        gProfile.laneChanges++;
        gProfile.consecutiveSameLane = 0;
    }
    else
    {
        gProfile.consecutiveSameLane++;
    }
    gProfile.lastLaneX = px;
}

void profileRegisterJump() { gProfile.jumpCount++; }
void profileRegisterRoll() { gProfile.rollCount++; }

static void resetProfile()
{
    gProfile.jumpCount = 0;
    gProfile.rollCount = 0;
    gProfile.laneChanges = 0;
    gProfile.sampleFrames = 0;
    gProfile.consecutiveSameLane = 0;
}

static ObstacleType adaptiveObstacleType()
{
    int frames = gProfile.sampleFrames;
    if (frames < 60)
        return (ObstacleType)(rand() % 3);

    float jumpRate = (float)gProfile.jumpCount / frames;
    float rollRate = (float)gProfile.rollCount / frames;
    float stayRate = (float)gProfile.consecutiveSameLane / frames;

    if (jumpRate > 0.05f)
        return LOW_BARRIER;

    if (rollRate > 0.05f)
        return SPEED_BUMP;

    if (stayRate > 0.6f)
        return BUS;

    return (ObstacleType)(rand() % 3);
}

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
static float playerEffectiveTopY() { return isRolling() ? 0.55f : playerTopY(); }

static const float LANES[3] = {-7.0f, 0.0f, 7.0f};

static float nearestLane(float x)
{
    float best = LANES[0];
    for (float l : LANES)
        if (fabs(x - l) < fabs(x - best))
            best = l;
    return best;
}

static float playerLane() { return nearestLane(getPlayerX()); }

static float differentLane(float fromX)
{
    float opts[2];
    int n = 0;
    for (float l : LANES)
        if (fabs(l - fromX) > 1.0f)
            opts[n++] = l;
    return opts[rand() % n];
}

static void updateChaser(Obstacle &obs, float speed)
{
    if (!obs.isChaser)
        return;

    float px = playerLane();

    if (obs.chaserCooldown > 0)
    {
        obs.chaserCooldown--;
        return;
    }

    switch (obs.chaserState)
    {

    case CHASER_IDLE:
        if (obs.z > -300.0f && obs.z < 5.0f)
        {
            if (fabs(px - obs.x) < 1.0f)
            {
                obs.chaserState = CHASER_TRACKING;
                obs.chaserCooldown = 5;
            }
            else
            {
                obs.chaserState = CHASER_SWITCHING;
                obs.chaserOriginX = obs.x;
                obs.chaserTargetX = px;
                obs.chaserCooldown = 5;
            }
        }
        break;

    case CHASER_TRACKING:
        obs.ownSpeed = speed * 0.5f;

        if (fabs(px - obs.x) > 1.0f)
        {
            obs.chaserState = CHASER_SWITCHING;
            obs.chaserOriginX = obs.x;
            obs.chaserTargetX = px;
            obs.chaserCooldown = 5;
        }
        break;

    case CHASER_SWITCHING:
    {

        if (fabs(px - obs.chaserTargetX) > 1.0f)
            obs.chaserTargetX = px;

        float dx = obs.chaserTargetX - obs.x;
        if (fabs(dx) < 0.3f)
        {
            obs.x = obs.chaserTargetX;
            obs.chaserState = CHASER_TRACKING;
            obs.chaserCooldown = 5;
        }
        else
        {

            float moveSpeed = fmaxf(0.08f, fabs(dx) * 0.04f);
            obs.x += (dx > 0 ? 1.0f : -1.0f) * moveSpeed;
        }
        break;
    }

    case CHASER_RETREATING:
    {
        float dx = obs.chaserOriginX - obs.x;
        if (fabs(dx) < 0.3f)
        {
            obs.x = obs.chaserOriginX;
            obs.ownSpeed = 0.0f;
            obs.chaserState = CHASER_IDLE;
            obs.chaserCooldown = 60;
        }
        else
        {
            obs.x += (dx > 0 ? 1.0f : -1.0f) * 0.08f;
        }
        break;
    }
    }

    if (obs.z > 8.0f && obs.chaserState != CHASER_IDLE)
    {
        obs.chaserState = CHASER_RETREATING;
        obs.ownSpeed = 0.0f;
        obs.chaserCooldown = 30;
    }
}

BusType randomBusType()
{
    int r = rand() % 100;
    if (r < 60)
        return BUS_401;
    if (r < 90)
        return BUS_365;
    return BUS_FANTASMAO;
}

static void createChaserBus(float z)
{

    float startX = differentLane(playerLane());

    Obstacle obs;
    obs.x = startX;
    obs.z = z;
    obs.type = BUS;
    obs.busType = BUS_FANTASMAO;
    obs.moving = true;
    obs.ownSpeed = 0.0f;
    obs.isChaser = true;
    obs.chaserState = CHASER_IDLE;
    obs.chaserTargetX = startX;
    obs.chaserOriginX = startX;
    obs.chaserCooldown = 0;
    obstacles.push_back(obs);
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
    obs.isChaser = false;
    obs.chaserState = CHASER_IDLE;
    obs.chaserTargetX = x;
    obs.chaserOriginX = x;
    obs.chaserCooldown = 0;
    obstacles.push_back(obs);
}

void clearNearbyObstacles(float z)
{
    for (auto &obs : obstacles)
        if (fabs(obs.z - z) < 8.0f)
            obs.z = 100.0f;
}

void createObstacleAt(float x, float z, ObstacleType type)
{
    Obstacle obs;
    obs.x = x;
    obs.z = z;
    obs.type = type;
    obs.moving = false;
    obs.ownSpeed = 0.0f;
    obs.isChaser = false;
    obs.chaserState = CHASER_IDLE;
    obs.chaserTargetX = x;
    obs.chaserOriginX = x;
    obs.chaserCooldown = 0;

    if (type == BUS)
        obs.busType = randomBusType();

    if (type == SPEED_BUMP && rand() % 100 < 40)
        spawnCoinArc(obs.x, obs.z);

    if (type == LOW_BARRIER && rand() % 100 < 70)
        spawnCoinArcLow(obs.x, obs.z);

    obstacles.push_back(obs);
}

static float obstacleSpacing(float speed)
{
    float minGap = speed * 60.0f * 3.0f;
    return fmaxf(80.0f, minGap);
}

bool hasObstacleNear(float x, float z, float radiusX, float radiusZ)
{
    for (auto &obs : obstacles)
        if (obs.type == BUS)
            if (fabs(x - obs.x) < radiusX && fabs(z - obs.z) < radiusZ)
                return true;
    return false;
}

void createObstacle(float z)
{
    ObstacleType adaptType = adaptiveObstacleType();

    int lane = rand() % 3;

    if (adaptType == BUS)
    {
        int pattern = rand() % 3;
        if (pattern == 0)
        {
            createBusAt(LANES[lane], z, rand() % 100 < 35);
        }
        else if (pattern == 1)
        {
            int free = rand() % 3;
            for (int i = 0; i < 3; i++)
                if (i != free)
                    createBusAt(LANES[i], z, false);
        }
        else
        {
            createBusAt(LANES[0], z, rand() % 100 < 35);
            createBusAt(LANES[2], z, rand() % 100 < 35);
        }
    }

    else if (adaptType == SPEED_BUMP)
    {

        float playerFavoredLane = nearestLane(getPlayerX());
        int pattern = rand() % 3;

        if (pattern == 0)
        {

            createObstacleAt(playerFavoredLane, z, SPEED_BUMP);
        }
        else if (pattern == 1)
        {
            int free = rand() % 3;
            for (int i = 0; i < 3; i++)
                if (i != free)
                    createObstacleAt(LANES[i], z, SPEED_BUMP);
        }
        else
        {
            for (int i = 0; i < 3; i++)
                createObstacleAt(LANES[i], z, SPEED_BUMP);
        }
    }

    else
    {
        int pattern = rand() % 3;
        if (pattern == 0)
            createObstacleAt(LANES[lane], z, LOW_BARRIER);
        else if (pattern == 1)
        {
            int free = rand() % 3;
            for (int i = 0; i < 3; i++)
                if (i != free)
                    createObstacleAt(LANES[i], z, LOW_BARRIER);
        }
        else
            for (int i = 0; i < 3; i++)
                createObstacleAt(LANES[i], z, LOW_BARRIER);
    }
}

void initObstacles()
{
    obstacleCount = 5;
    nextObstacleIncrease = 5000;
    nextRecycleZ = -120.0f;
    obstacles.clear();
    resetProfile();
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

            if (obs.isChaser)
                glColor3f(1.0f, 0.4f, 0.4f);

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

            if (obs.isChaser)
                glColor3f(1.0f, 1.0f, 1.0f);

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
                lowBarrierModel->draw();
                glPopMatrix();
            }
        }
    }
}

static int chaserSpawnThreshold = 1000;
static bool chaserActive = false;

void updateObstacles(float speed, float score)
{
    float spacing = obstacleSpacing(speed);

    updateProfile();

    if (gProfile.sampleFrames >= 300)
        resetProfile();

    if (obstacles.empty())
        for (int i = 0; i < obstacleCount; i++)
            createObstacle(-120.0f - i * spacing);

    if (!chaserActive && score >= chaserSpawnThreshold)
    {
        createChaserBus(-150.0f);
        chaserActive = true;
        chaserSpawnThreshold += 5000;
    }

    if (score >= nextObstacleIncrease)
    {
        obstacleCount = (int)(obstacleCount * 1.2f);
        nextObstacleIncrease *= 2;

        obstacles.erase(
            remove_if(obstacles.begin(), obstacles.end(),
                      [](const Obstacle &o)
                      { return o.z > 15.0f; }),
            obstacles.end());

        float farthestZ = -200.0f;
        for (auto &obs : obstacles)
            farthestZ = fminf(farthestZ, obs.z);

        for (int i = 0; i < obstacleCount; i++)
            createObstacle(farthestZ - (i + 1) * spacing);

        nextRecycleZ = farthestZ - obstacleCount * spacing;
    }

    for (auto &obs : obstacles)
    {
        obs.z += speed;

        if (obs.type == BUS)
        {
            if (obs.moving || obs.isChaser)
                obs.z += obs.ownSpeed;

            if (obs.isChaser)
                updateChaser(obs, speed);
        }

        if (obs.z > 15.0f && !obs.isChaser)
        {
            int lane = rand() % 3;
            obs.x = LANES[lane];
            obs.z = nextRecycleZ;
            nextRecycleZ -= spacing;

            if (nextRecycleZ < -600.0f)
                nextRecycleZ = -spacing * obstacleCount;

            int newKind = rand() % 3;
            if (newKind == 0)
            {
                obs.type = BUS;
                obs.busType = randomBusType();
                obs.moving = (rand() % 100 < 10);
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

        if (obs.isChaser && obs.z > 30.0f)
        {
            obs.z = -160.0f;
            obs.x = differentLane(playerLane());
            obs.chaserState = CHASER_IDLE;
            obs.chaserCooldown = 60;
            obs.ownSpeed = 0.0f;
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
            if (distX < hb.hitX && distZ < hb.hitZ)
            {
                bool jumpedOver = (isRolling() ? 0.05f : playerBottomY()) >= hb.hitH;
                if (!jumpedOver)
                {
                    if (isFinalExamActive())
                    {
                        consumeFinalExam();
                        triggerShieldFlash();
                        clearNearbyObstacles(obs.z);
                        return;
                    }
                    playHitFrontAnimation();
                    startGameOverSequence();
                    return;
                }
            }
        }

        if (obs.type == SPEED_BUMP)
        {
            const float bumpHalfX = 1.5f, bumpHalfZ = 2.0f, bumpH = 0.6f;
            if (distX < bumpHalfX && distZ < bumpHalfZ && pbY < bumpH)
            {
                if (isFinalExamActive())
                {
                    consumeFinalExam();
                    triggerShieldFlash();
                    clearNearbyObstacles(obs.z);
                    return;
                }
                playHitFrontAnimation();
                startGameOverSequence();
                return;
            }
        }

        if (obs.type == LOW_BARRIER)
        {
            const float barrierHalfX = 2.0f, barrierHalfZ = 1.2f, barrierTopY = 1.8f;
            if (distX < barrierHalfX && distZ < barrierHalfZ)
            {
                bool rolledUnder = isRolling();
                bool jumpedOver = playerBottomY() >= barrierTopY;
                if (!rolledUnder && !jumpedOver)
                {
                    if (isFinalExamActive())
                    {
                        consumeFinalExam();
                        triggerShieldFlash();
                        clearNearbyObstacles(obs.z);
                        return;
                    }
                    playHitFrontAnimation();
                    startGameOverSequence();
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
                if (distX < hb.hitX && distZ < hb.hitZ && ptY < hb.hitH)
                {
                    if (sideHitWarning)
                    {
                        playHitFrontAnimation();
                        startGameOverSequence();
                    }
                    else
                    {
                        sideHitWarning = true;
                        sideHitTimer = 0;
                        int dir = targetX > getPlayerX() ? 1 : -1;
                        playSideBumpAnimation(dir);

                        showProfessorNear(); // faz o professor voltar a perseguir
                    }

                    return false;
                }
            }
        }
    }
    return true;
}