#include "draw.h"
#include <math.h>

#define _USE_MATH_DEFINES
#ifndef PI
#define PI M_PI
#endif

void init(void)
{
    /* RGBA = (1,1,1,0) 白色背景 */
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    /* 用黑色，线宽 2 像素 */
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);

    /*屏幕外框*/
    glRect(-0.6, -0.4, 0.6, 0.4, GL_LINE_LOOP);

    /*屏幕内框 */
    glRectSmooth(-0.55, -0.35, 0.55, 0.35, 0.05, GL_LINE_LOOP);

    /*圆角矩形 */
    glRectSmooth(-0.25, -0.62, 0.25, -0.47, 0.03, GL_LINE_LOOP);

    /*去除基座上边多余的那一段线 */
    glColor3f(1.0f, 1.0f, 1.0f);
    glLine(-0.16, -0.47, 0.16, -0.47);

    /* 支架 */
    glColor3f(0.0f, 0.0f, 0.0f);
    glRect(-0.16, -0.55, 0.16, -0.40, GL_LINE_LOOP);

    /* 三角形  */
    glTri(-0.15, -0.10, /* 左下 */
          0.15, -0.10,  /* 右下 */
          0.0, 0.15,    /* 顶点 */
          GL_LINE_LOOP);

    /* 小圆*/
    glArc(0.0, 0.0, 0.0, 2.0 * PI, 0.05, GL_LINE_STRIP);

    glFlush();
}

/* 线 */
void glLine(double x1, double y1, double x2, double y2)
{
    glBegin(GL_LINE_STRIP);
    glVertex2f((GLfloat)x1, (GLfloat)y1);
    glVertex2f((GLfloat)x2, (GLfloat)y2);
    glEnd();
}

/* 三角形*/
void glTri(double x1, double y1,
           double x2, double y2,
           double x3, double y3,
           int mode)
{
    glBegin(mode);
    glVertex2f((GLfloat)x1, (GLfloat)y1);
    glVertex2f((GLfloat)x2, (GLfloat)y2);
    glVertex2f((GLfloat)x3, (GLfloat)y3);
    glEnd();
}

/* 矩形*/
void glRect(double leftX, double leftY,
            double rightX, double rightY,
            int mode)
{
    glBegin(mode);
    /* 左下 */
    glVertex2f((GLfloat)leftX, (GLfloat)leftY);
    /* 右下 */
    glVertex2f((GLfloat)rightX, (GLfloat)leftY);
    /* 右上 */
    glVertex2f((GLfloat)rightX, (GLfloat)rightY);
    /* 左上 */
    glVertex2f((GLfloat)leftX, (GLfloat)rightY);
    glEnd();
}

/* 只生成圆弧上的一串散点 */
void glArcPoint(double x, double y,
                double start_angle, double end_angle,
                double radius)
{
    double delta_angle = PI / 180.0;
    double angle;

    for (angle = start_angle; angle <= end_angle + 1e-9; angle += delta_angle)
    {
        double vx = x + radius * cos(angle);
        double vy = y + radius * sin(angle);
        glVertex2f((GLfloat)vx, (GLfloat)vy);
    }
}

/* 画弧线*/
void glArc(double x, double y,
           double start_angle, double end_angle,
           double radius, int mode)
{
    glBegin(mode);
    glArcPoint(x, y, start_angle, end_angle, radius);
    glEnd();
}

/* 圆角矩形*/
void glRectSmooth(double leftX, double leftY,
                  double rightX, double rightY,
                  double radius, int mode)
{
    double s_leftX = leftX + radius;
    double s_leftY = leftY + radius;
    double s_rightX = rightX - radius;
    double s_rightY = rightY - radius;

    glBegin(mode);

    glArcPoint(s_leftX, s_leftY, PI, 1.5 * PI, radius);
    glArcPoint(s_rightX, s_leftY, 1.5 * PI, 2.0 * PI, radius);
    glArcPoint(s_rightX, s_rightY, 0.0, 0.5 * PI, radius);
    glArcPoint(s_leftX, s_rightY, 0.5 * PI, PI, radius);

    glEnd();
}
