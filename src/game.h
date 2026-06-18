#ifndef GAME_H
#define GAME_H

void initGame();
void drawGame();
void updateGame(int value);
void gameKeyboard(unsigned char key, int x, int y);
void drawText2D(float x, float y, const char *text);
void checkCollision();

#endif