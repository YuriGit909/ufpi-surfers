#ifndef POWERUP_H
#define POWERUP_H

void initPowerUps();
void drawPowerUps();
void updatePowerUps(float speed);
void checkPowerUps();

bool isDoublePointsActive();
void initPowerUpModels();
bool isFinalExamActive();
void activateFinalExam();
void consumeFinalExam();
#endif