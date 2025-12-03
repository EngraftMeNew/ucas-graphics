#include <vector>
#include <algorithm>
#include <cmath>

#include "draw.h"
#include "color.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

// 窗口大小
int g_winWidth = 600;
int g_winHeight = 600;

// 拖拽
bool g_isDragging = false;

// 鼠标拖拽起点 / 当前点（世界坐标系：-1~1）
double g_startX = 0.0, g_startY = 0.0;
double g_curX = 0.0, g_curY = 0.0;

// 形状类型
enum ShapeType
{
    SHAPE_POINT = 0,
    SHAPE_LINE,
    SHAPE_TRIANGLE,
    SHAPE_RECT,
    SHAPE_CIRCLE,
    SHAPE_ROUNDRECT
};

enum DrawMode
{
    MODE_LINE = 0, // 只画线
    MODE_FILL      // 填充颜色 + 黑色轮廓
};

// 保存已绘制图形的信息
struct Shape
{
    ShapeType type;
    Color3f color;
    bool filled;

    // 通用参数
    double x1, y1;
    double x2, y2;
    double x3, y3;
    double r; // 圆半径 / 圆角半径等
};

// 当前选中的形状  颜色  模式
ShapeType g_currentShape = SHAPE_LINE;
Color3f g_currentColor = {0.0f, 0.0f, 0.0f}; // 默认黑色
DrawMode g_currentMode = MODE_LINE;

// 记录所有已绘制图形
std::vector<Shape> g_shapes;

// --- 工具函数：屏幕坐标 -> 世界坐标（-1 ~ 1） ---
double screenToWorldX(int x)
{
    return (double)x / (double)g_winWidth * 2.0 - 1.0;
}
double screenToWorldY(int y)
{
    // GLUT 的 y=0 在窗口顶部，这里翻转一下
    return 1.0 - (double)y / (double)g_winHeight * 2.0;
}

// 绘制图形
void drawShape(const Shape &s)
{
    switch (s.type)
    {
    case SHAPE_POINT:
        // 点
        setColor(&s.color);
        glPointSize(4.0f);
        glBegin(GL_POINTS);
        glVertex2f((GLfloat)s.x1, (GLfloat)s.y1);
        glEnd();
        break;

    case SHAPE_LINE:
        // 线条
        setColor(&s.color);
        glLine(s.x1, s.y1, s.x2, s.y2);
        break;

    case SHAPE_TRIANGLE:
        if (s.filled)
        {
            // 先用颜色填充，再用黑线描边
            setColor(&s.color);
            glTri(s.x1, s.y1, s.x2, s.y2, s.x3, s.y3, GL_TRIANGLES);
            glColor3f(0.0f, 0.0f, 0.0f);
            glTri(s.x1, s.y1, s.x2, s.y2, s.x3, s.y3, GL_LINE_LOOP);
        }
        else
        {
            // 只画线
            setColor(&s.color);
            glTri(s.x1, s.y1, s.x2, s.y2, s.x3, s.y3, GL_LINE_LOOP);
        }
        break;

    case SHAPE_RECT:
    {
        double left = std::min(s.x1, s.x2);
        double right = std::max(s.x1, s.x2);
        double bottom = std::min(s.y1, s.y2);
        double top = std::max(s.y1, s.y2);

        if (s.filled)
        {
            setColor(&s.color);
            glRect(left, bottom, right, top, GL_POLYGON);
            glColor3f(0.0f, 0.0f, 0.0f);
            glRect(left, bottom, right, top, GL_LINE_LOOP);
        }
        else
        {
            setColor(&s.color);
            glRect(left, bottom, right, top, GL_LINE_LOOP);
        }
        break;
    }

    case SHAPE_CIRCLE:
        if (s.filled)
        {
            setColor(&s.color);
            glDisk(s.x1, s.y1, s.r);
            glColor3f(0.0f, 0.0f, 0.0f);
            glArc(s.x1, s.y1, 0.0, 2.0 * PI, s.r, GL_LINE_STRIP);
        }
        else
        {
            setColor(&s.color);
            glArc(s.x1, s.y1, 0.0, 2.0 * PI, s.r, GL_LINE_STRIP);
        }
        break;

    case SHAPE_ROUNDRECT:
    {
        double left = std::min(s.x1, s.x2);
        double right = std::max(s.x1, s.x2);
        double bottom = std::min(s.y1, s.y2);
        double top = std::max(s.y1, s.y2);
        double radius = s.r;

        if (s.filled)
        {
            setColor(&s.color);
            glRectSmooth(left, bottom, right, top, radius, GL_POLYGON);
            glColor3f(0.0f, 0.0f, 0.0f);
            glRectSmooth(left, bottom, right, top, radius, GL_LINE_LOOP);
        }
        else
        {
            setColor(&s.color);
            glRectSmooth(left, bottom, right, top, radius, GL_LINE_LOOP);
        }
        break;
    }

    default:
        break;
    }
}

// 根据拖拽起点/终点生成一个 Shape
Shape makeShapeFromDrag(ShapeType type,
                        const Color3f &color,
                        double x1, double y1,
                        double x2, double y2)
{
    Shape s{};
    s.type = type;
    s.color = color;
    s.filled = (g_currentMode == MODE_FILL);

    if (type == SHAPE_POINT)
    {
        // 点只用终点
        s.x1 = x2;
        s.y1 = y2;
        return s;
    }

    if (type == SHAPE_LINE)
    {
        s.x1 = x1;
        s.y1 = y1;
        s.x2 = x2;
        s.y2 = y2;
        return s;
    }

    double left = std::min(x1, x2);
    double right = std::max(x1, x2);
    double bottom = std::min(y1, y2);
    double top = std::max(y1, y2);

    switch (type)
    {
    case SHAPE_TRIANGLE:
        s.x1 = left;
        s.y1 = bottom;
        s.x2 = right;
        s.y2 = bottom;
        s.x3 = (left + right) / 2.0;
        s.y3 = top;
        break;

    case SHAPE_RECT:
        s.x1 = left;
        s.y1 = bottom;
        s.x2 = right;
        s.y2 = top;
        break;

    case SHAPE_CIRCLE:
    {
        double cx = (x1 + x2) / 2.0;
        double cy = (y1 + y2) / 2.0;
        double dx = right - left;
        double dy = top - bottom;
        double r = std::min(std::fabs(dx), std::fabs(dy)) / 2.0;
        s.x1 = cx;
        s.y1 = cy;
        s.r = r;
        break;
    }

    case SHAPE_ROUNDRECT:
    {
        s.x1 = left;
        s.y1 = bottom;
        s.x2 = right;
        s.y2 = top;
        // 圆角半径取宽高较小值的 10%
        double w = right - left;
        double h = top - bottom;
        s.r = 0.1 * std::min(std::fabs(w), std::fabs(h));
        break;
    }

    default:
        break;
    }

    return s;
}

void display_cb(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    // 画历史图形
    for (const auto &s : g_shapes)
    {
        drawShape(s);
    }

    // 画正在拖拽的预览
    if (g_isDragging)
    {
        Shape preview = makeShapeFromDrag(
            g_currentShape, g_currentColor, g_startX, g_startY, g_curX, g_curY);
        drawShape(preview);
    }

    glFlush();
}

// 窗口尺寸变化
void reshape_cb(int w, int h)
{
    g_winWidth = (w > 1) ? w : 1;
    g_winHeight = (h > 1) ? h : 1;
    glViewport(0, 0, w, h);
}

// 鼠标左键绘图
void mouse_cb(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            g_isDragging = true;
            g_startX = g_curX = screenToWorldX(x);
            g_startY = g_curY = screenToWorldY(y);
        }
        else if (state == GLUT_UP && g_isDragging)
        {
            g_isDragging = false;
            double endX = screenToWorldX(x);
            double endY = screenToWorldY(y);

            Shape s = makeShapeFromDrag(
                g_currentShape, g_currentColor,
                g_startX, g_startY, endX, endY);

            g_shapes.push_back(s);
        }

        glutPostRedisplay();
    }
}

// 鼠标移动
void motion_cb(int x, int y)
{
    if (g_isDragging)
    {
        g_curX = screenToWorldX(x);
        g_curY = screenToWorldY(y);
        glutPostRedisplay();
    }
}

// 菜单 
enum
{
    MENU_CLEAR = 1,

    MENU_SHAPE_POINT = 10,
    MENU_SHAPE_LINE,
    MENU_SHAPE_TRIANGLE,
    MENU_SHAPE_RECT,
    MENU_SHAPE_CIRCLE,
    MENU_SHAPE_ROUNDRECT,

    MENU_COLOR_BLACK = 100,
    MENU_COLOR_RED,
    MENU_COLOR_GREEN,
    MENU_COLOR_BLUE,
    MENU_COLOR_YELLOW,
    MENU_COLOR_MAGENTA,
    MENU_COLOR_CYAN,
    MENU_COLOR_GRAY,

    MENU_MODE_LINE = 200,
    MENU_MODE_FILL
};

void shape_menu_cb(int value)
{
    switch (value)
    {
    case MENU_SHAPE_POINT:
        g_currentShape = SHAPE_POINT;
        break;
    case MENU_SHAPE_LINE:
        g_currentShape = SHAPE_LINE;
        break;
    case MENU_SHAPE_TRIANGLE:
        g_currentShape = SHAPE_TRIANGLE;
        break;
    case MENU_SHAPE_RECT:
        g_currentShape = SHAPE_RECT;
        break;
    case MENU_SHAPE_CIRCLE:
        g_currentShape = SHAPE_CIRCLE;
        break;
    case MENU_SHAPE_ROUNDRECT:
        g_currentShape = SHAPE_ROUNDRECT;
        break;
    default:
        break;
    }
}

void color_menu_cb(int value)
{
    switch (value)
    {
    case MENU_COLOR_BLACK:
        g_currentColor = {0.0f, 0.0f, 0.0f};
        break;
    case MENU_COLOR_RED:
        g_currentColor = {1.0f, 0.0f, 0.0f};
        break;
    case MENU_COLOR_GREEN:
        g_currentColor = {0.0f, 1.0f, 0.0f};
        break;
    case MENU_COLOR_BLUE:
        g_currentColor = {0.0f, 0.0f, 1.0f};
        break;
    case MENU_COLOR_YELLOW:
        g_currentColor = {1.0f, 1.0f, 0.0f};
        break;
    case MENU_COLOR_MAGENTA:
        g_currentColor = {1.0f, 0.0f, 1.0f};
        break;
    case MENU_COLOR_CYAN:
        g_currentColor = {0.0f, 1.0f, 1.0f};
        break;
    case MENU_COLOR_GRAY:
        g_currentColor = {0.5f, 0.5f, 0.5f};
        break;
    default:
        break;
    }
}

void mode_menu_cb(int value)
{
    switch (value)
    {
    case MENU_MODE_LINE:
        g_currentMode = MODE_LINE;
        break;
    case MENU_MODE_FILL:
        g_currentMode = MODE_FILL;
        break;
    default:
        break;
    }
}

void main_menu_cb(int value)
{
    if (value == MENU_CLEAR)
    {
        g_shapes.clear();
        glutPostRedisplay();
    }
}

// 创建右键菜单
void create_menus()
{
    int shapeMenu = glutCreateMenu(shape_menu_cb);
    glutAddMenuEntry("Point", MENU_SHAPE_POINT);
    glutAddMenuEntry("Line", MENU_SHAPE_LINE);
    glutAddMenuEntry("Triangle", MENU_SHAPE_TRIANGLE);
    glutAddMenuEntry("Rectangle", MENU_SHAPE_RECT);
    glutAddMenuEntry("Circle", MENU_SHAPE_CIRCLE);
    glutAddMenuEntry("Round Rect", MENU_SHAPE_ROUNDRECT);

    int colorMenu = glutCreateMenu(color_menu_cb);
    glutAddMenuEntry("Black", MENU_COLOR_BLACK);
    glutAddMenuEntry("Red", MENU_COLOR_RED);
    glutAddMenuEntry("Green", MENU_COLOR_GREEN);
    glutAddMenuEntry("Blue", MENU_COLOR_BLUE);
    glutAddMenuEntry("Yellow", MENU_COLOR_YELLOW);
    glutAddMenuEntry("Magenta", MENU_COLOR_MAGENTA);
    glutAddMenuEntry("Cyan", MENU_COLOR_CYAN);
    glutAddMenuEntry("Gray", MENU_COLOR_GRAY);

    int modeMenu = glutCreateMenu(mode_menu_cb);
    glutAddMenuEntry("Line", MENU_MODE_LINE);
    glutAddMenuEntry("Fill", MENU_MODE_FILL);

    int mainMenu = glutCreateMenu(main_menu_cb);
    glutAddSubMenu("Choose Shape", shapeMenu);
    glutAddSubMenu("Choose Color", colorMenu);
    glutAddSubMenu("Mode", modeMenu);
    glutAddMenuEntry("Clear Screen", MENU_CLEAR);

    // 右键弹出
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(g_winWidth, g_winHeight);
    glutCreateWindow("Mouse Drawing");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    init();

    // 注册回调
    glutDisplayFunc(display_cb);
    glutReshapeFunc(reshape_cb);
    glutMouseFunc(mouse_cb);
    glutMotionFunc(motion_cb);

    // 创建右键菜单
    create_menus();

    glutMainLoop();
    return 0;
}
