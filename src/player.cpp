#include <GL/glew.h>
#include <GL/glut.h>
#include "player.h"
#include "animated_model.h"
#include "animation.h"
#include "animator.h"

static AnimatedModel *character = nullptr;
static Animation *runAnimation = nullptr;
static Animation *jumpAnimation = nullptr;
static Animation *rollAnimation = nullptr;
static Animator *animator = nullptr;

void initPlayerModel()
{
    character = new AnimatedModel("./assets/player/estudante.glb");

    runAnimation = new Animation("./assets/player/estudante.glb", character, 0);
    jumpAnimation = new Animation("./assets/player/estudante.glb", character, 1);
    rollAnimation = new Animation("./assets/player/estudante.glb", character, 2);

    animator = new Animator(runAnimation);
    animator->playAnimation(runAnimation, true);
}

float lanes[3] = {-7.0f, 0.0f, 7.0f};
int currentLane = 1;

float playerY = 1.0f;
float velocityY = 0.0f;
bool jumping = false;

const float gravity = -0.06f;
const float jumpForce = 0.60f;
bool rolling = false;
int rollTimer = 0;

const int ROLL_DURATION = 35; // ~0,5s

void drawPlayer()
{
    glPushMatrix();

    glTranslatef(
        lanes[currentLane],
        (isRolling() ? 0.3f : playerY) + 0.6f,
        2.0f);

    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

    if (character)
    {
        glScalef(0.1f, 0.1f, 0.1f);

        if (animator)
            character->draw(animator->getFinalBoneMatrices());
        else
            character->draw();
    }
    else
    {
        if (isRolling())
            glScalef(1.4f, 0.5f, 1.0f);

        glColor3f(0.0f, 0.0f, 1.0f);
        glutSolidCube(1.0f);
    }

    glPopMatrix();
}

void roll()
{
    if (!rolling && !jumping)
    {
        rolling = true;
        rollTimer = ROLL_DURATION;

        if (animator && rollAnimation)
        {
            animator->setSpeed(2.5f);
            animator->playAnimation(rollAnimation, false);
        }
    }
}

bool isRolling()
{
    return rolling;
}

void jump()
{
    if (!jumping && !rolling)
    {
        velocityY = jumpForce;
        jumping = true;

        if (animator && jumpAnimation)
        {
            animator->setSpeed(1.5f);
            animator->playAnimation(jumpAnimation, false);
        }
    }
}

void updatePlayer()
{
    if (jumping)
    {
        velocityY += gravity;
        playerY += velocityY;

        if (playerY <= 1.0f)
        {
            playerY = 1.0f;
            velocityY = 0.0f;
            jumping = false;

            if (animator && runAnimation)
            {
                animator->setSpeed(1.0f);
                animator->playAnimation(runAnimation, true);
            }
        }
    }

    if (rolling)
    {
        rollTimer--;

        if (rollTimer <= 0)
        {
            rolling = false;

            if (animator && runAnimation)
            {
                animator->setSpeed(1.0f);
                animator->playAnimation(runAnimation, true);
            }
        }
    }

    if (animator)
        animator->updateAnimation(0.016f);
}

void movePlayerLeft()
{
    if (currentLane > 0)
    {
        currentLane--;
    }
}

void movePlayerRight()
{
    if (currentLane < 2)
    {
        currentLane++;
    }
}

float getPlayerX()
{
    return lanes[currentLane];
}

float getPlayerY()
{
    return playerY;
}