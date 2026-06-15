#include <GL/glut.h>
#include "game/game.h"

/* 
g++ src/main.cpp \
src/game/game.cpp \
src/ui/menu.cpp \
src/ui/text.cpp \
src/utils/draw.cpp \
-o main \
-lGL -lGLU -lglut 
*/
int main(int argc, char** argv) {

    glutInit(&argc, argv); // inicializa o GLUT

    glutInitDisplayMode(
        GLUT_DOUBLE |     // usa dois buffers para evitar tela piscando
        GLUT_RGB |        // usa cores RGB
        GLUT_DEPTH        // usa profundidade 3D
    );

    glutInitWindowSize(800, 600);              // tamanho da janela
    glutCreateWindow("Subway UFPI");          // título da janela

    initGame();                               // configura OpenGL

    glutDisplayFunc(display);                 // função que desenha
    glutKeyboardFunc(keyboard);               // função do teclado
    glutTimerFunc(16, update, 0);             // função de atualização

    glutMainLoop();                           // inicia o loop principal

    return 0;
}