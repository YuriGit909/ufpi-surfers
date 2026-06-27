#include <GL/glew.h>
#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "powerup.h"
#include "player.h"
#include "model.h"

Model* mangaModel = nullptr;

using namespace std;

void initPowerUpModels()
{
    mangaModel = new Model("./assets/models/manga.obj");
}

enum PowerType
{
    DOUBLE_POINTS,
    SPEED_BOOST,
    INVINCIBILITY
};

struct PowerUp
{
    float x;
    float z;
    bool active;
    int respawnTimer;
    PowerType type;
};

vector<PowerUp> powerUps;

bool doublePointsActive = false;
int doublePointsTimer = 0;

const int DOUBLE_POINTS_DURATION = 600;

void drawPowerUps()
{
    for (auto &p : powerUps)
    {
        if (!p.active)
            continue;

        switch (p.type)
        {
        case DOUBLE_POINTS:

            glPushMatrix();

glTranslatef(p.x, 1.2f, p.z);

// ajuste depois conforme necessário
glScalef(1.0f, 1.0f, 1.0f);

if (mangaModel)
    mangaModel->draw();

glPopMatrix();

            break;

        default:
            break;
        }
    }
}

void checkPowerUps()
{
    float playerX = getPlayerX();

    for (auto &p : powerUps)
    {
        if (!p.active)
            continue;

        float distanceX = fabs(playerX - p.x);
        float distanceZ = fabs(2.0f - p.z);

        if (distanceX < 1.0f && distanceZ < 1.0f)
        {
            p.active = false;
            p.respawnTimer = 600;

            switch (p.type)
            {
            case DOUBLE_POINTS:

                doublePointsActive = true;
                doublePointsTimer = DOUBLE_POINTS_DURATION;

                printf("Manga coletada! Pontos x2\n");

                break;

            default:
                break;
            }
        }
    }
}

void initPowerUps()
{
    powerUps.clear();

    PowerUp manga;

    manga.x = 0.0f;
    manga.z = -100.0f;
    manga.active = true;
    manga.type = DOUBLE_POINTS;
    manga.respawnTimer = 0;

    powerUps.push_back(manga);

    doublePointsActive = false;
    doublePointsTimer = 0;
}

void updatePowerUps(float speed)
{
    for (auto &p : powerUps)
    {
        if (!p.active)
        {
            p.respawnTimer--;

            if (p.respawnTimer <= 0)
            {
                int lane = rand() % 3;

                if (lane == 0) p.x = -7.0f;
                if (lane == 1) p.x = 0.0f;
                if (lane == 2) p.x = 7.0f;

                p.z = -250.0f;
                p.active = true;
            }

            continue;
        }

        p.z += speed;

        if (p.z > 5.0f)
        {
            p.active = false;
            p.respawnTimer = 1200;
        }
    }

    if (doublePointsActive)
    {
        doublePointsTimer--;

        if (doublePointsTimer <= 0)
        {
            doublePointsActive = false;
        }
    }
}

bool isDoublePointsActive()
{
    return doublePointsActive;
}