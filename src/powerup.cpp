#include <GL/glew.h>
#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include "powerup.h"
#include "player.h"
#include "model.h"
#include "menu.h"

bool finalExamActive = false;
int finalExamTimer = 0;
const int FINAL_EXAM_DURATION = 60 * 60; // 1 minuto em 60 FPS


bool finalExamCollectedOnce = false;

GLuint finalExamTexture = 0;
Model *mangaModel = nullptr;

using namespace std;

void activateFinalExam()
{
    finalExamActive = true;
    finalExamTimer = FINAL_EXAM_DURATION;
    finalExamCollectedOnce = true;
}

bool isFinalExamActive()
{
    return finalExamActive;
}

void consumeFinalExam()
{
    finalExamActive = false;
    finalExamTimer = 0;
}

void initPowerUpModels()
{
    mangaModel = new Model("./assets/models/manga.obj");
    finalExamTexture = loadMenuTexture("./assets/textures/prova-final.png");
}

enum PowerType
{
    DOUBLE_POINTS,
    SPEED_BOOST,
    INVINCIBILITY,
    FINAL_EXAM
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
        case FINAL_EXAM:

    glPushMatrix();

    glTranslatef(p.x, 1.3f, p.z);

    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, finalExamTexture);
    glColor4f(1,1,1,1);

    glScalef(2.0f, 3.0f, 1.0f);


    glBegin(GL_QUADS);

        glTexCoord2f(0,1); glVertex3f(-0.5f,-0.5f,0);
        glTexCoord2f(1,1); glVertex3f( 0.5f,-0.5f,0);
        glTexCoord2f(1,0); glVertex3f( 0.5f, 0.5f,0);
        glTexCoord2f(0,0); glVertex3f(-0.5f, 0.5f,0);

    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);

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

            case FINAL_EXAM:
                activateFinalExam();
                printf("Prova final coletada! Escudo ativo\n");
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

                if (lane == 0)
                    p.x = -7.0f;
                if (lane == 1)
                    p.x = 0.0f;
                if (lane == 2)
                    p.x = 7.0f;

                int r = rand() % 100;

                if (!finalExamCollectedOnce)
                {
                    if (r < 100)
                        p.type = FINAL_EXAM;
                    else
                        p.type = DOUBLE_POINTS;
                }
                else
                {
                    if (r < 5)
                        p.type = FINAL_EXAM;
                    else
                        p.type = DOUBLE_POINTS;
                }

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

    if (finalExamActive)
    {
        finalExamTimer--;

        if (finalExamTimer <= 0)
        {
            finalExamActive = false;
            finalExamTimer = 0;
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