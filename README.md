# OpenGL Composition Framework  
C++98 / OpenGL / Multi‑Pass Rendering Framework

このリポジトリは、OpenGL における  
**FBO → Texture → Screen**  
および  
**Multi‑Pass / Ping‑Pong / Multi‑Texture Composition**  
を最小限の C++98 構造で安全に構築するためのフレームワークです。

LiquidFun の Composition クラスに着想を得ており、  
複雑なレンダリングパイプラインを **ノードを並べるだけで組み立てられる** ことを目的としています。

---

## 1. ビルド方法（Linux / macOS / MinGW）

### Linux / macOS

```bash
g++ -std=c++98 main.cpp Composition.cpp -lGL -lGLEW -lglfw -o demo
Windows（MinGW）
bash
g++ -std=c++98 main.cpp Composition.cpp -lopengl32 -lglew32 -lglfw3 -o demo.exe
※ GLEW / GLFW / GLAD のセットアップは環境に応じて行ってください。

2. フレームワーク構成
RenderTarget  
FBO + Texture + Depth の最小構成

ShaderPass  
GLSL プログラムと uniform セット

FullscreenQuad  
ポストエフェクト用のフルスクリーンクアッド

CompositionNode  
input → output を shader で処理する 1 パス

CompositionGraph  
ノードを順に実行するだけのシンプルなグラフ

3. 代表的なパターン（レシピ集）
A. FBO → Texture → Screen
cpp
CompositionGraph graph;

CompositionNode pass;
pass.input  = &sceneFBO;
pass.output = NULL;
pass.shader = &postShader;

graph.add(pass);
graph.run(quad);
B. FBO → Screen → FBO → Screen（ブラー）
cpp
graph.add( CompositionNode{ &sceneFBO, &blurFBO, &blurX } );
graph.add( CompositionNode{ &blurFBO,  &sceneFBO, &blurY } );
graph.add( CompositionNode{ &sceneFBO, NULL,      &composite } );

graph.run(quad);
C. Ping‑Pong（FBO A ↔ FBO B）
cpp
for (int i = 0; i < iterations; ++i) {
    graph.add( CompositionNode{ &fboA, &fboB, &step } );
    graph.add( CompositionNode{ &fboB, &fboA, &step } );
}
graph.run(quad);
D. Multi‑Texture 合成（Glow / SSAO）
cpp
CompositionNode node;
node.input  = &sceneFBO;
node.output = NULL;
node.shader = &composite;

node.addExtraTexture("uBlur", blurFBO.texture, 1);
node.addExtraTexture("uMask", maskFBO.texture, 2);

graph.add(node);
graph.run(quad);
4. ディレクトリ構成
コード
OpenGL-composition-framework/
│
├── README.md
├── Composition.h
├── Composition.cpp
├── main.cpp
│
└── shaders/
    ├── fullscreen.vert
    ├── passthrough.frag
    ├── blurX.frag
    ├── blurY.frag
    └── composite.frag
5. ライセンス
MIT License

6. 今後の拡張予定
MRT（Multiple Render Targets）対応

Compute Shader 版（OpenGL 4.3+）

Android / EGL / JNI 版

9 TextureView 用 Multi‑Target Graph