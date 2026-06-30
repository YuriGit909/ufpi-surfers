#ifndef GAME_H
#define GAME_H

enum Screen {
    MENU,
    INTRO,
    PLAYING
};

extern Screen currentScreen;

void initGame();
void drawGame();
void updateGame(int value);
void gameKeyboard(unsigned char key, int x, int y);
void initGameModels();
void triggerGameOverWithAnimation();
void startGameOverSequence();

extern int windowWidth;
extern int windowHeight;

#endif