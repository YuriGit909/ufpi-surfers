#include <GL/glut.h>
#include <cmath>
#include "menu.h"
#include "camera.h"
#include "model.h"
#include "external/stb_image.h"

GLuint titleTexture = 0;
GLuint cloudTexture = 0;
GLuint startButtonTexture = 0;
GLuint exitButtonTexture = 0;

Model portal("./assets/models/ufpi.obj");

GLuint loadMenuTexture(const char *file)
{
    int w, h, c;

    stbi_set_flip_vertically_on_load(false);

    unsigned char *data = stbi_load(file, &w, &h, &c, 4);

    if (!data)
    {
        printf("Erro ao carregar %s\n", file);
        return 0;
    }

    GLuint tex;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        w,
        h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    return tex;
}


void initMenu()
{
    titleTexture = loadMenuTexture("./assets/textures/UFPI-SURFERS.png");
    cloudTexture = loadMenuTexture("./assets/textures/cloud.png");

    startButtonTexture = loadMenuTexture("./assets/textures/start_button.png");
    exitButtonTexture  = loadMenuTexture("./assets/textures/exit_button.png");
}

void drawImage(GLuint texture, float cx, float cy, float w, float h)
{
    if (texture == 0)
        return;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texture);
    glColor4f(1, 1, 1, 1);

    float x1 = cx - w / 2.0f;
    float x2 = cx + w / 2.0f;
    float y1 = cy - h / 2.0f;
    float y2 = cy + h / 2.0f;

    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(x1, y1);
        glTexCoord2f(1, 1); glVertex2f(x2, y1);
        glTexCoord2f(1, 0); glVertex2f(x2, y2);
        glTexCoord2f(0, 0); glVertex2f(x1, y2);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void drawCloud(float x, float y, float w, float h)
{
    if (cloudTexture == 0)
        return;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, cloudTexture);
    glColor4f(1, 1, 1, 0.85f);

    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(x,     y + h);
        glTexCoord2f(1,0); glVertex2f(x + w, y + h);
        glTexCoord2f(1,1); glVertex2f(x + w, y);
        glTexCoord2f(0,1); glVertex2f(x,     y);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void drawStrokeText(float x, float y, float scale, const char *text)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, scale);

    for (int i = 0; text[i] != '\0'; i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);
    }
    glPopMatrix();
}

void setupIntroCamera(float cameraZ)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0, 1000.0 / 600.0, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float cameraX = -2.0f - (18.0f - cameraZ) * 0.25f;

    gluLookAt(
        cameraX, 0.8f, cameraZ,
        -9.5f, 0.3f, -20.0f,
        0.0f, 1.0f, 0.0f);
}

void drawBitmapText(float x, float y, const char *text)
{
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

void drawMenuBox(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void drawSkyGradient()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1000, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);
        glColor3f(0.05f, 0.25f, 0.75f); // topo azul forte
        glVertex2f(0, 600);
        glVertex2f(1000, 600);

        glColor3f(0.55f, 0.80f, 1.0f); // horizonte claro
        glVertex2f(1000, 0);
        glVertex2f(0, 0);
    glEnd();

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void drawSunRays()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1000, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float cx = 500.0f;
    float cy = 720.0f;

   for (int i = -10; i <= 10; i++)
    {
        float x = cx + i * 120.0f;

        glBegin(GL_TRIANGLES);

            glColor4f(1.0f, 1.0f, 0.85f, 0.06f);
            glVertex2f(cx, cy);

            glColor4f(1.0f, 1.0f, 0.85f, 0.06f);
            glVertex2f(x - 25.0f, 0);

            glColor4f(1.0f, 1.0f, 0.85f, 0.06f);
            glVertex2f(x + 25.0f, 0);

        glEnd();
    }

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void drawMenuIntro(float cameraZ)
{
    drawSkyGradient();
    drawSunRays();

    // Nuvens no fundo
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1000, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    drawCloud(20, 400, 180, 140);
    drawCloud(760, 450, 220, 140);
    drawCloud(330, 450, 140, 140);
    drawCloud(560, 470, 120, 140);

    glEnable(GL_DEPTH_TEST);

    // Portal por cima das nuvens
    setupIntroCamera(cameraZ);

    glPushMatrix();
    glTranslatef(0.0f, -2.0f, -8.0f);
    glScalef(0.35f, 0.35f, 0.35f);
    glRotatef(-92.0f, 0.0f, 1.0f, 0.0f);
    portal.draw();
    glPopMatrix();
}

void drawMenu()
{

    
    drawSkyGradient();
    drawSunRays();
   
    glClearColor(0.45f, 0.75f, 1.0f, 1.0f);

    setupMenuCamera();

    // Portal
    glPushMatrix();
    glTranslatef(0.0f, -2.0f, -8.0f);
    glScalef(0.35f, 0.35f, 0.35f);
    glRotatef(-92.0f, 0.0f, 1.0f, 0.0f);
    portal.draw();
    glPopMatrix();

    // Interface 2D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1000, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    drawCloud(20, 400, 180, 140);
    drawCloud(760, 450, 220, 140);
    drawCloud(330, 450, 140, 140);
    drawCloud(560, 470, 120, 140);

    glEnable(GL_TEXTURE_2D);
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

glBindTexture(GL_TEXTURE_2D, titleTexture);
glColor4f(1, 1, 1, 1);

float cx = 500.0f;
float cy = 500.0f;
float w = 1000.0f;   // largura
float h = 400.0f;   // altura

glBegin(GL_QUADS);

    glTexCoord2f(0,0);
    glVertex2f(cx - w/2, cy + h/2);

    glTexCoord2f(1,0);
    glVertex2f(cx + w/2, cy + h/2);

    glTexCoord2f(1,1);
    glVertex2f(cx + w/2, cy - h/2);

    glTexCoord2f(0,1);
    glVertex2f(cx - w/2, cy - h/2);

glEnd();


glDisable(GL_BLEND);
glDisable(GL_TEXTURE_2D);
    // Caixa do botão iniciar
   drawImage(titleTexture, 500, 500, 1000, 400);

drawImage(startButtonTexture, 475, 150, 350, 60);
drawImage(exitButtonTexture, 475, 82, 260, 52);
    glEnable(GL_DEPTH_TEST);
}