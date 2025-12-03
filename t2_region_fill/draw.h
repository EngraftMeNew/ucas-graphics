#pragma once

#define GLUT_DISABLE_ATEXIT_HACK

#include <windows.h>
#include <GL/glut.h>

void init(void);
void display(void);

/* 画线：两点坐标 */
void glLine(double x1, double y1, double x2, double y2);

/* 画三角形：三个点坐标，mode: GL_LINE_LOOP / GL_POLYGON */
void glTri(double x1, double y1,
           double x2, double y2,
           double x3, double y3,
           int mode);

/* 画矩形：左下角 (leftX,leftY)，右上角 (rightX,rightY) */
void glRect(double leftX, double leftY,
            double rightX, double rightY,
            int mode);

/* 画弧线：圆心 (x,y)，起始/结束弧度，半径，mode */
void glArc(double x, double y,
           double start_angle, double end_angle,
           double radius, int mode);

/* 生成弧线上的散点 */
void glArcPoint(double x, double y,
                double start_angle, double end_angle,
                double radius);

/* 圆角矩形：四条边用 4 段 1/4 圆弧拼起来 */
void glRectSmooth(double leftX, double leftY,
                  double rightX, double rightY,
                  double radius, int mode);

void glDisk(double x, double y, double radius);