#include <GL/glut.h>

void setupMenuCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(60.0, 1000.0 / 600.0, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

gluLookAt(
    -4.0f, 2.6f, 11.5f,
    -4.0f, 1.7f, -2.0f,
    0.0f, 1.0f, 0.0f
);

}