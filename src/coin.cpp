#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>

#include "coin.h"
#include "player.h"

using namespace std;

struct Coin
{
    float x;
    float z;
    bool active;
};

vector<Coin> coins;

int ruCoins = 0;
int nextCoinSpawnScore = 20; // primeira fileira vem depois de 150 pontos
bool coinLineActive = false;

void spawnCoinLine()
{
    coins.clear();

    int lane = rand() % 3;

    float x = 0.0f;

    if (lane == 0) x = -3.0f;
    if (lane == 1) x = 0.0f;
    if (lane == 2) x = 3.0f;

    for (int i = 0; i < 6; i++)
    {
        Coin c;

        c.x = x;
        c.z = -80.0f - i * 3.0f;
        c.active = true;

        coins.push_back(c);
    }

    coinLineActive = true;
}

void initCoins()
{
    coins.clear();
    ruCoins = 0;
    nextCoinSpawnScore = 20;
    coinLineActive = false;
}

void updateCoins(float speed, float score)
{
    if (!coinLineActive && score >= nextCoinSpawnScore)
    {
        spawnCoinLine();

        // próximo conjunto só depois de mais pontos
        nextCoinSpawnScore += 20;
    }

    for (auto &c : coins)
    {
        if (!c.active)
            continue;

        c.z += speed;

        if (c.z > 5.0f)
        {
            c.active = false;
        }
    }

    bool anyActive = false;

    for (auto &c : coins)
    {
        if (c.active)
        {
            anyActive = true;
            break;
        }
    }

    if (!anyActive)
    {
        coinLineActive = false;
        coins.clear();
    }
}

void drawCoins()
{
    glColor3f(1.0f, 0.9f, 0.0f);

    for (auto &c : coins)
    {
        if (!c.active)
            continue;

        glPushMatrix();
            glTranslatef(c.x, 1.0f, c.z);
            glScalef(0.45f, 0.45f, 0.15f);
            glutSolidSphere(1.0f, 20, 20);
        glPopMatrix();
    }
}

void checkCoinCollision()
{
    float playerX = getPlayerX();

    for (auto &c : coins)
    {
        if (!c.active)
            continue;

        float distanceX = fabs(playerX - c.x);
        float distanceZ = fabs(2.0f - c.z);

        if (distanceX < 1.0f && distanceZ < 1.0f)
        {
            c.active = false;
            ruCoins++;
        }
    }
}

int getRuCoins()
{
    return ruCoins;
}