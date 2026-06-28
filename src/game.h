#ifndef GAME_H
#define GAME_H

void initGame();
void drawGame();
void updateGame(int value);
void gameKeyboard(unsigned char key, int x, int y);
void initGameModels();
extern int windowWidth;
extern int windowHeight;

#endif