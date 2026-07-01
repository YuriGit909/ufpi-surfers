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
const int FINAL_EXAM_DURATION = 60 * 15; 
static int   shieldFlashTimer    = 0;
static const int SHIELD_FLASH_DURATION = 60;   // ~1 s a 60 FPS
static int   shieldFlashPhase    = 0;           
static float hudPulse = 0.0f;   
GLuint mangaHudTexture = 0;     
bool finalExamCollectedOnce = false;

GLuint finalExamTexture = 0;
Model *mangaModel = nullptr;

using namespace std;


void triggerShieldFlash()
{
    shieldFlashTimer = SHIELD_FLASH_DURATION;
    shieldFlashPhase = 0;
}

void drawShieldFlash()
{
    if (shieldFlashTimer <= 0) return;

    // alterna branco/vermelho a cada 10 frames → 3 piscadas em 1 s
    int segment = (SHIELD_FLASH_DURATION - shieldFlashTimer) / 10;
    shieldFlashPhase = segment % 2;

    float alpha = (float)shieldFlashTimer / SHIELD_FLASH_DURATION * 0.55f;

    // salva projeção e entra em modo 2-D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (shieldFlashPhase == 0)
        glColor4f(1.0f, 1.0f, 1.0f, alpha);   // branco
    else
        glColor4f(1.0f, 0.1f, 0.1f, alpha);   // vermelho

    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(1, 0);
        glVertex2f(1, 1);
        glVertex2f(0, 1);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    shieldFlashTimer--;
}

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
    mangaHudTexture = loadMenuTexture("./assets/textures/manga.png"); 
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

static void drawHUDIcon(float screenX, float screenY, float size,
                        GLuint tex,
                        float borderR, float borderG, float borderB,
                        float pulse)
{
    float border = 3.0f + 2.0f * sinf(pulse);   // 1..5 px pulsante

    // -- borda --
    glColor4f(borderR, borderG, borderB, 0.9f);
    glBegin(GL_QUADS);
        glVertex2f(screenX - border,        screenY - border);
        glVertex2f(screenX + size + border, screenY - border);
        glVertex2f(screenX + size + border, screenY + size + border);
        glVertex2f(screenX - border,        screenY + size + border);
    glEnd();

    // -- ícone --
    if (tex)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
        glColor4f(1, 1, 1, 1);
        glBegin(GL_QUADS);
            glTexCoord2f(0,1); glVertex2f(screenX,        screenY);
            glTexCoord2f(1,1); glVertex2f(screenX + size, screenY);
            glTexCoord2f(1,0); glVertex2f(screenX + size, screenY + size);
            glTexCoord2f(0,0); glVertex2f(screenX,        screenY + size);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    else
    {
        // fallback colorido (manga sem textura 2D)
        glColor4f(1.0f, 0.55f, 0.0f, 1.0f);   // laranja
        glBegin(GL_QUADS);
            glVertex2f(screenX,        screenY);
            glVertex2f(screenX + size, screenY);
            glVertex2f(screenX + size, screenY + size);
            glVertex2f(screenX,        screenY + size);
        glEnd();
    }
}

void drawPowerUpHUD(int screenW, int screenH)
{
    if (!finalExamActive && !doublePointsActive) return;

    hudPulse += 0.08f;
    if (hudPulse > 6.2832f) hudPulse = 0.0f;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, screenW, 0, screenH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float SIZE    = 56.0f;
    const float MARGIN  = 16.0f;
    const float BAR_H   = 8.0f;   // altura da barra
    const float BAR_GAP = 4.0f;   // espaço entre ícone e barra
    const float SLOT_H  = SIZE + BAR_GAP + BAR_H + 8.0f; // altura total de cada slot
    float slot = 0;

    // Lambda: desenha ícone + barra de duração
    auto drawSlot = [&](GLuint tex,
                        float bR, float bG, float bB,
                        float progress,       // 0.0 a 1.0
                        float barR, float barG, float barB)
    {
        float iconX = screenW - SIZE - MARGIN;
        float iconY = screenH - SIZE - MARGIN - slot * SLOT_H;

        // Ícone com borda
        drawHUDIcon(iconX, iconY, SIZE, tex, bR, bG, bB, hudPulse);

        // Fundo da barra (cinza escuro)
        float barY = iconY - BAR_GAP - BAR_H;
        glColor4f(0.2f, 0.2f, 0.2f, 0.8f);
        glBegin(GL_QUADS);
            glVertex2f(iconX,          barY);
            glVertex2f(iconX + SIZE,   barY);
            glVertex2f(iconX + SIZE,   barY + BAR_H);
            glVertex2f(iconX,          barY + BAR_H);
        glEnd();

        // Preenchimento da barra
        float fillW = SIZE * progress;
        // pisca vermelho nos últimos 10%
        if (progress < 0.1f) {
            float flicker = sinf(hudPulse * 6.0f) > 0 ? 1.0f : 0.4f;
            glColor4f(1.0f, 0.1f, 0.1f, flicker);
        } else {
            glColor4f(barR, barG, barB, 1.0f);
        }
        glBegin(GL_QUADS);
            glVertex2f(iconX,          barY);
            glVertex2f(iconX + fillW,  barY);
            glVertex2f(iconX + fillW,  barY + BAR_H);
            glVertex2f(iconX,          barY + BAR_H);
        glEnd();

        slot++;
    };

    if (finalExamActive)
    {
        float progress = (float)finalExamTimer / FINAL_EXAM_DURATION;
        bool  lastTen  = finalExamTimer < 60 * 10;

        float bR, bG, bB;
        if (lastTen) {
            float p = sinf(hudPulse * 3.0f);
            bR = 1.0f; bG = 0.1f * (0.5f + 0.5f * p); bB = 0.0f;
        } else {
            bR = 1.0f; bG = 0.75f + 0.15f * sinf(hudPulse); bB = 0.0f;
        }

        drawSlot(finalExamTexture, bR, bG, bB, progress, 1.0f, 0.8f, 0.0f);
    }

    if (doublePointsActive)
    {
        float progress = (float)doublePointsTimer / DOUBLE_POINTS_DURATION;
        bool  lastTen  = doublePointsTimer < 60 * 10;

        float bR, bG, bB;
        if (lastTen) {
            float p = sinf(hudPulse * 3.0f);
            bR = 1.0f; bG = 0.1f * (0.5f + 0.5f * p); bB = 0.0f;
        } else {
            bR = 0.2f + 0.1f * sinf(hudPulse); bG = 0.9f; bB = 0.2f;
        }

        drawSlot(mangaHudTexture, bR, bG, bB, progress, 0.2f, 0.9f, 0.2f);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

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