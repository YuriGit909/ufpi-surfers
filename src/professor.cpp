#include <GL/glew.h>
#include <GL/glut.h>

#include "professor.h"
#include "player.h"
#include "animated_model.h"
#include "animation.h"
#include "animator.h"

static AnimatedModel* professor = nullptr;
static Animation* professorRun = nullptr;
static Animator* professorAnimator = nullptr;

static bool professorVisible = true;

static float professorDistance = 2.5f;
static float professorTargetDistance = 10.0f;

static float professorAlpha = 1.0f;
static float professorTargetAlpha = 1.0f;

static bool professorIntroMode = true;

static Animation* professorPullAnimation = nullptr;

void initProfessor()
{
    professor = new AnimatedModel(
    "./assets/player/professor.glb",
    "./assets/player/professor/textures"
    );

    professorRun = new Animation("./assets/player/professor.glb", professor, 1);
    professorPullAnimation = new Animation("./assets/player/professor.glb", professor, 0);

    professorAnimator = new Animator(professorRun);
    professorAnimator->playAnimation(professorRun, true);

    professorIntroMode = true;
    professorVisible = true;
    professorDistance = 2.5f;
    professorTargetDistance = 10.0f;
    professorAlpha = 1.0f;
    professorTargetAlpha = 0.0f;
}


void showProfessorCapture()
{
    professorVisible = true;
    professorIntroMode = false;

    professorDistance = 6.0f;
    professorTargetDistance = 2.0f;

    professorAlpha = 0.0f;
    professorTargetAlpha = 1.0f;
}

void playProfessorPullAnimation()
{
    if (professorAnimator && professorPullAnimation)
    {
        professorAnimator->setSpeed(1.0f);
        professorAnimator->playAnimation(professorPullAnimation, false);
    }
}

void updateProfessor(float deltaTime)
{
    if (professorAnimator)
        professorAnimator->updateAnimation(deltaTime);

    float fadeSpeed = professorIntroMode ? 0.005f : 0.05f;
    float distanceSpeed = professorIntroMode ? 0.01f : 0.05f;

    professorDistance += (professorTargetDistance - professorDistance) * distanceSpeed;
    professorAlpha += (professorTargetAlpha - professorAlpha) * fadeSpeed;

    if (professorAlpha < 0.02f)
    {
        professorAlpha = 0.0f;
        professorVisible = false;
    }
}

void drawProfessor()
{
    if (!professorVisible || !professor || professorAlpha <= 0.0f)
        return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, professorAlpha);

    glPushMatrix();

    glTranslatef(
    getPlayerX(),
    1.2f,
    0.3 + professorDistance
);

    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glScalef(0.1f, 0.1f, 0.1f);

    if (professorAnimator)
        professor->draw(professorAnimator->getFinalBoneMatrices());
    else
        professor->draw();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);

    glPopMatrix();

}

void showProfessorNear()
{
    professorIntroMode = false;
    professorVisible = true;
    professorTargetDistance = 2.5f;
    professorTargetAlpha = 1.0f;
}

void hideProfessor()
{
    professorTargetDistance = 10.0f;
    professorTargetAlpha = 0.0f;
}

void resetProfessorAnimation()
{
    if (professorAnimator && professorRun)
    {
        professorAnimator->setSpeed(1.0f);
        professorAnimator->playAnimation(professorRun, true);
    }

    resetProfessorIntro();
}

void resetProfessorIntro()
{
    professorIntroMode = true;
    professorVisible = true;

    professorDistance = 2.5f;
    professorTargetDistance = 10.0f;

    professorAlpha = 1.0f;
    professorTargetAlpha = 0.0f;
}