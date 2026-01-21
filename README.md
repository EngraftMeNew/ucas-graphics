#  中国科学院大学《计算机图形学基础》习题3——光线追踪
---


这是一个光线追踪渲染器，完成了以下功能：
- 发光球体作为光源
- 反射 + 折射 + 递归终止条件
- 正方体物体
- 相机运动动画（逐帧输出）+ GIF 动图导出
- 静态截图与动画帧均输出 PNG

## 运行环境
- 宿主机：Windows 11
- 运行/编译平台：WSL2 Ubuntu 22.04 LTS
- 编译器：g++
- 构建工具：GNU Make
- 编译选项：-std=c++17 -O2 -Wall -Wextra
  
## 编译

在 `...\Homework3`目录下:

```
make clean && make
```

## 运行

默认运行
```
./raytracer
```

可选 （快速渲染模式，更快，画质/采样更低）:

```
./raytracer --quick
```




## 输出文件（Outputs）

所有输出均生成在 `output/` 目录下：

* `output/stills/shot_0.png` .. `shot_3.png`
  （4 个不同相机位置的静态截图）
* `output/frames/frame_XXXX.png`
  （动画帧序列）
* `output/animation.gif`
  （相机绕场景运动的动画 GIF）

**特别说明：**

* 整体背景为***纯黑色***

---

## 代码结构（Code Structure）

* `core.h`：数学常量、`Vec3`、`Ray`、`Material` 以及辅助函数
* `objects.h`：`Object` 基类与 `Sphere` / `Plane` / `Box`（正方体）等对象
* `scene.h`：场景管理（物体列表）与求交/阴影判断
* `camera.h`：相机模型与逐像素生成主射线
* `render.h` / `render.cpp`：递归 `trace()` 与渲染主循环
* `image_io.h` / `image_io.cpp`：PNG 写入与 GIF 编码
* `main.cpp`：场景搭建与输出流程（截图 + 动画）

---

## 光照与着色细节（Lighting & Shading）

### 光源（发光球体）

* 位置：**(0.0, 4.5, -5.0)**
* 半径：**0.7**
* 发光强度/颜色：**(4.0, 4.0, 4.0)**

实现方式：将发光球体作为普通物体加入场景列表；直接光照通过对该光源发射阴影光线计算遮挡，并做漫反射。

### 着色模型

* **直接漫反射**：`baseColor * max(0, N·L) * light.emission`
* **反射（递归）**：使用 Fresnel 项与反射方向计算递归贡献
* **折射（递归）**：使用 IOR（折射率）计算折射方向并递归
* **环境项**：`baseColor * 0.08`（避免未照亮区域过黑）

### 地面材质

* 地面为 **纯黑漫反射**，且 **不参与反射**（reflection = 0）
* 因此地面不会“镜面反光”

### 背景

* 画面为纯黑色

---


## 推荐运行流程（Linux / WSL）

```bash
cd path-to-Homework3
make
./raytracer
```

如需更高分辨率：请不要使用 `--quick`，并在 `main.cpp` 中调整相关常量（例如分辨率、采样数 spp、最大递归深度等）。


---

## 结果展示
![gif](output/animation.gif)

`注意：这里放的是动图，在PDF中会被固定为静态,如需查看动图，请打开Homework3/output/animation.gif`

## 工具说明

本项目参考课程提供的示例代码完成核心算法实现；同时借助Codex 用于辅助生成部分模板/样板代码与重构建议。场景搭建、求交与递归追踪逻辑、输出规范等均由本人完成实现与验证。