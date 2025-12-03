# 任务一：直线与圆弧绘制（小电脑图案）

## 1. 开发与运行环境

- 操作系统：Windows 11
- 编译器：MSYS2 UCRT64 下的 g++（x86_64-w64-mingw32）
- 图形库：OpenGL + freeglut
- 开发工具：VS Code

依赖安装（示例）：
- MSYS2 中安装：
  - `pacman -S mingw-w64-ucrt-x86_64-gcc`
  - `pacman -S mingw-w64-ucrt-x86_64-freeglut`

## 2. 文件说明

- `main.c`  
  程序入口，负责：
  - 初始化 GLUT（窗口大小 600×600，坐标系 -1~1）
  - 调用 `init()` 设置背景色
  - 注册 `display()` 为显示回调

- `draw.h`  
  图形绘制相关函数的声明：
  - `init`, `display`
  - `glLine`, `glTri`, `glRect`
  - `glArc`, `glArcPoint`, `glRectSmooth`

- `draw.c`  
  图形绘制函数的具体实现：
  - 使用 OpenGL 原语封装基本图元：
    - `glLine`：用 `GL_LINE_STRIP` 绘制一条线段
    - `glTri`：用 `GL_LINE_LOOP` / `GL_POLYGON` 绘制三角形
    - `glRect`：用 `GL_LINE_LOOP` 绘制矩形
    - `glArcPoint`：以步长 `PI/180` 计算圆弧上的散点坐标
    - `glArc`：在 `glBegin/glEnd` 中调用 `glArcPoint` 绘制圆弧
    - `glRectSmooth`：用 4 段 1/4 圆弧拼成圆角矩形
  - 在 `display()` 中调用上述函数，绘制小电脑图案：
    - 屏幕外框：矩形 `[-0.6,-0.4] ~ [0.6,0.4]`
    - 屏幕内框：圆角矩形 `[-0.55,-0.35] ~ [0.55,0.35]`，圆角半径 `0.05`
    - 基座：圆角矩形 `[-0.25,-0.62] ~ [0.25,-0.47]`，圆角半径 `0.03`
    - 支架：矩形 `[-0.16,-0.55] ~ [0.16,-0.40]`
    - 屏幕内图案：
      - 三角形顶点 `(-0.15,-0.1)，(0.15,-0.1)，(0,0.15)`
      - 圆：圆心 `(0,0)`，半径 `0.05`，整圆 `0~2π`

- `Makefile`  
  使用 `mingw32-make` 编译的配置，生成 `main.exe`。

- `build.bat`  
  Windows 下双击编译的批处理脚本，内部调用 g++ 生成 `main.exe`。

- `screenshot.png`  
  程序运行后的窗口截图，作为实验结果展示。

## 3. 编译与运行方法

### 方法一：使用 Makefile（推荐）

在 MSYS2 UCRT64 终端或 PowerShell 中进入目录：

```bash
cd path/to/t1_line_circle
mingw32-make
./main.exe
```

### 方法二：使用build.bat

在资源管理器中双击build.bat,编译成功后同目录下生成 main.exe，再双击运行或在终端中执行：main.exe

## 参考与致谢

本作业部分图形构型与参数设置参考了以下开源项目

- [Jia040223 / UCAS-Computer-Graphic](https://github.com/Jia040223/UCAS-Computer-Graphic)
  - 主要参考了其中小电脑图案的构图思路（屏幕、支架、底座、三角形与圆的相对位置与大小）