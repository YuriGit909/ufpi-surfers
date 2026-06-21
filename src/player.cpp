#include <GL/glut.h>
#include "player.h"


float lanes[3] = {-7.0f, 0.0f, 7.0f};
int currentLane = 1;

float playerY = 1.0f;
float velocityY = 0.0f;
bool jumping = false;

const float gravity = -0.06f;
const float jumpForce = 0.75f;

void drawPlayer() {
    glPushMatrix();

        glTranslatef(
            lanes[currentLane],
            playerY,
            2.0f
        );

        glColor3f(0.0f, 0.0f, 1.0f);

        glutSolidCube(1.0f);

    glPopMatrix();
}

void jump() {
    if (!jumping) {
        velocityY = jumpForce;
        jumping = true;
    }
}

void updatePlayer() {

    if (jumping) {

        velocityY += gravity;
        playerY += velocityY;

        if (playerY <= 1.0f) {
            playerY = 1.0f;
            velocityY = 0.0f;
            jumping = false;
        }
    }
}

void movePlayerLeft() {
    if (currentLane > 0) {
        currentLane--;
    }
}

void movePlayerRight() {
    if (currentLane < 2) {
        currentLane++;
    }
}

float getPlayerX() {
    return lanes[currentLane];
}

float getPlayerY() {
    return playerY;
}