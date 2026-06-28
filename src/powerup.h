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
void triggerShieldFlash();
void drawShieldFlash();   
extern GLuint mangaHudTexture;
void drawPowerUpHUD(int screenW, int screenH);     
#endif