// Bibliotecas utilizadas pelo OpenGL
#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/glut.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#define ESC 27

void init();
void display(void);

int main(int argc, char **argv){
    // Inicializa o GLUT e passa os argumentos do sistema
    glutInit(&argc, argv);

    // Configura o modo de exibicao
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

    // Configuracao da posicao e tamanho da janela
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);

    // Cria a janela
    glutCreateWindow("UFPI Surfers");

    init();

    // Definicao das funcoes de callback
    glutDisplayFunc(display);

    // Loop infinito do GLUT
    glutMainLoop();

    return EXIT_SUCCESS;
}

void init(void){
    glClearColor (1.0, 1.0, 1.0, 1.0); //Limpa a tela com a cor branca;
    glEnable(GL_DEPTH_TEST); // Habilita o algoritmo Z-Buffer

    // Iluminacao
    // glEnable(GL_LIGHTING);
    // glEnable(GL_LIGHT0);
    // glEnable(GL_COLOR_MATERIAL);
    // glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void display(void){
    // Limpa o buffer de cores
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Troca os buffers para exibir o que foi desenhado
    glutSwapBuffers();
}