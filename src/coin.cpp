#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include "coin.h"
#include "player.h"
#include "obstacle.h"
#include "menu.h"
using namespace std;

GLuint coinTexture = 0;

struct Coin
{
    float x;
    float z;
    float y;
    bool active;
};

vector<Coin> coins;

int ruCoins = 0;
int nextCoinSpawnScore = 20; // primeira fileira vem depois de 150 pontos
bool coinLineActive = false;

void spawnCoinArcLow(float x, float z)
{
    const int quantidade = 6;

    for (int i = 0; i < quantidade; i++)
    {
        Coin c;
        c.x = x;
        c.z = z - 4.0f + i * 1.6f;
        c.y = 0.3f; // rasteiro, abaixo da barreira
        c.active = true;
        coins.push_back(c);
    }
}

void spawnCoinLine()
{
    coins.clear();

    int lane = rand() % 3;
    float x = lane == 0 ? -7.0f : lane == 1 ? 0.0f
                                            : 7.0f;

    // Tenta outra faixa se tiver ônibus no caminho
    for (int tentativa = 0; tentativa < 3; tentativa++)
    {
        if (!hasObstacleNear(x, -80.0f, 2.0f, 15.0f))
            break;

        lane = (lane + 1) % 3;
        x = lane == 0 ? -7.0f : lane == 1 ? 0.0f
                                          : 7.0f;
    }

    for (int i = 0; i < 6; i++)
    {
        Coin c;
        c.x = x;
        c.y = 1.0f;
        c.z = -80.0f - i * 3.0f;
        c.active = true;
        coins.push_back(c);
    }

    coinLineActive = true;
}

void initCoins()
{

    coinTexture = loadMenuTexture("./assets/textures/ficha.png");
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
        nextCoinSpawnScore += 50;
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

    coins.erase(
        remove_if(coins.begin(), coins.end(),
                  [](Coin &c)
                  {
                      return !c.active;
                  }),
        coins.end());
}

void drawCoins()
{
    glColor3f(1.0f, 0.9f, 0.0f);

    for (auto &c : coins)
    {
        if (!c.active)
            continue;

        glPushMatrix();
        glTranslatef(c.x, c.y, c.z);
        glScalef(1.75, 1.5f, 1.5f);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindTexture(GL_TEXTURE_2D, coinTexture);

        glBegin(GL_QUADS);

        glTexCoord2f(0, 1); glVertex3f(-0.5f, -0.5f, 0);
        glTexCoord2f(1, 1); glVertex3f( 0.5f, -0.5f, 0);
        glTexCoord2f(1, 0); glVertex3f( 0.5f,  0.5f, 0);
        glTexCoord2f(0, 0); glVertex3f(-0.5f,  0.5f, 0);

        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }
}

void checkCoinCollision()
{
    float playerX = getPlayerX();
    float playerY = getPlayerY();
    float playerTop    = isRolling() ? 0.55f : playerY + 0.5f;
    float playerBottom = isRolling() ? 0.05f : playerY - 0.5f;

    for (auto &c : coins)
    {
        if (!c.active)
            continue;

        float distanceX = fabs(playerX - c.x);
        float distanceZ = fabs(2.0f - c.z);

        bool hitY = c.y >= playerBottom && c.y <= playerTop;

        if (distanceX < 1.0f && distanceZ < 1.0f && hitY)
        {
            c.active = false;
            ruCoins++;
        }
    }
}

void spawnCoinArc(float x, float z)
{
    const int quantidade = 6;

    for (int i = 0; i < quantidade; i++)
    {
        Coin c;

        c.x = x;

        float t = (float)i / (quantidade - 1);

        c.z = z - 4.0f + i * 1.6f;
        c.y = 1.2f + sin(t * 3.14159f) * 2.0f;
        c.active = true;

        coins.push_back(c);
    }
}

int getRuCoins()
{
    return ruCoins;
}