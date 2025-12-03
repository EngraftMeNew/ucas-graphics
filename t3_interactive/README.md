# 任务三：鼠标交互绘图系统（OpenGL + FreeGLUT）

本项目在前两项任务的基础上，实现了完整的**鼠标交互绘图功能**：

```text
- 右键菜单选择图形类型（点、直线、三角形、矩形、圆形、圆角矩形）
- 右键菜单选择绘图颜色（多种）
- 右键菜单选择绘图模式（线条模式 / 填充模式）
- 左键拖拽绘制图形  
  - 按下：起点  
  - 松开：终点  
- Clear Screen 清空屏幕  
- 可连续绘制多个图形  
- 所有基本图元（线、矩形、三角形、圆、圆角矩形）均复用上一实验的函数
```

---

## 1. 开发与运行环境

- **操作系统**：Windows 11  
- **编译器**：MSYS2 UCRT64 下的 g++（x86_64-w64-mingw32）  
- **图形库**：OpenGL + freeglut  
- **开发工具**：VS Code  

环境安装示例：

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-freeglut
```

## 2. 文件说明

```text
t3_mouse_draw/
│
├── main.cpp         # 程序入口：右键菜单 + 鼠标拖拽绘图逻辑
│
├── draw.c           # 图形绘制：直线、三角形、矩形、圆角矩形、圆弧等
├── draw.h
│
├── color.c          # 颜色模块：预定义颜色 + setColor 颜色工具
├── color.h
|
└── Makefile         # mingw32-make 编译配置
```

## 3. 编译与运行方法

```bash
cd path/to/t3_interactive
mingw32-make
./main.exe
```

## 4. 使用说明

### 1. 右键打开菜单

右键单击窗口，会显示：

Choose Shape

Choose Color

Mode

Clear Screen

### 2. 选择绘制形状

Choose Shape → Line / Rectangle / Circle ...

### 3. 选择颜色

Choose Color → Red / Blue / Green ...

### 4. 选择绘制模式

Choose Mode → Line（仅轮廓）

Choose Mode → Fill（实心 + 黑色轮廓）

### 5. 左键拖拽绘制

按下左键：确定起点

拖动：实时显示当前图形

抬起左键：生成最终图形
