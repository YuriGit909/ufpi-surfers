#ifndef GAME_H
#define GAME_H

// Estados possíveis do jogo
enum GameState {
    MENU,       // tela inicial
    PLAYING,    // jogo rodando
    GAME_OVER   // jogador perdeu
};

extern GameState gameState; // variável global do estado atual

void initGame();            // inicializa OpenGL
void display();             // desenha a tela
void update(int value);     // atualiza o jogo
void keyboard(unsigned char key, int x, int y); // lê teclado

#endif