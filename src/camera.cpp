#include <GL/glut.h>

void setupMenuCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0, 1000.0 / 600.0, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
        -1, 3, 10,   // posição da câmera
        -1, 3, 0,    // para onde olha
        0, 1, 0     // eixo de cima
    );
}