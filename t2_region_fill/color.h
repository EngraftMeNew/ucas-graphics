#ifndef COLOR_H
#define COLOR_H

#include <GL/glut.h>

typedef struct
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
} Color3f;


extern const Color3f color_bg;
extern const Color3f color_border;
extern const Color3f color_screen;
extern const Color3f color_triangle;
extern const Color3f color_circle;
extern const Color3f color_stand;

static inline void setColor(const Color3f *c)
{
    glColor3f(c->r, c->g, c->b);
}

#endif
