#define GLUT_DISABLE_ATEXIT_HACK
#include <windows.h> // 仅 Windows 系统
#include <GL/glut.h>

void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glOrtho(-5, 5, -5, 5, 5, 15);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0, 0, 10,
              0, 0, 0,
              0, 1, 0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glutWireTeapot(3); // 画线框茶壶
    glFlush();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(400, 400);
    glutCreateWindow("OpenGL Test: Teapot");

    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
