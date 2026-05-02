# OpenGL-cheat-sheet
For offscreen rendering using Framebuffer Object (FBO) in OpenGL

# OpenGL Composition Framework  
C++98 / OpenGL / Multi‑Pass Rendering Framework

このリポジトリは、OpenGL における  
**FBO → Texture → Screen**  
および  
**Multi‑Pass / Ping‑Pong / Multi‑Texture Composition**  
を最小限の C++98 構造で安全に組み立てるためのフレームワークです。

LiquidFun の Composition クラスに着想を得ており、  
複雑なレンダリングパイプラインを **ノードを並べるだけで構築**できます。

---

# 1. ビルド方法（Linux / macOS / MinGW）

## Linux / macOS

```bash
g++ -std=c++98 main.cpp Composition.cpp -lGL -lGLEW -lglfw -o demo
