#ifndef POWERUP_H
#define POWERUP_H
extern GLuint mangaHudTexture;

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
void drawPowerUpHUD(int screenW, int screenH);     
#endif