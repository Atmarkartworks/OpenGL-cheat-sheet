# OpenGL Composition Framework

C++98 / OpenGL / Multi-Pass Rendering Framework

This repository is a minimal C++98 framework for safely building
**FBO -> Texture -> Screen** and
**Multi-Pass / Ping-Pong / Multi-Texture Composition** pipelines in OpenGL.

Inspired by LiquidFun's Composition class,
it aims to let you build complex rendering pipelines just by arranging nodes.

---

## 1. Build

### Linux / macOS

```bash
g++ -std=c++98 main.cpp Composition.cpp -lGL -lGLEW -lglfw -o demo
```

### Windows (MinGW)

```bash
g++ -std=c++98 main.cpp Composition.cpp -lopengl32 -lglew32 -lglfw3 -o demo.exe
```

Note: Set up GLEW / GLFW / GLAD according to your environment.

## 2. Framework Components

- RenderTarget: Minimal set of FBO + Texture + Depth.
- ShaderPass: GLSL program plus uniform helpers.
- FullscreenQuad: Fullscreen quad for post-processing.
- CompositionNode: One pass that processes input -> output using a shader.
- CompositionGraph: Simple graph that runs nodes sequentially.

## 3. Typical Patterns (Recipes)

### A. FBO -> Texture -> Screen

```cpp
CompositionGraph graph;

CompositionNode pass;
pass.input  = &sceneFBO;
pass.output = NULL;
pass.shader = &postShader;

graph.add(pass);
graph.run(quad);
```

### B. FBO -> Screen -> FBO -> Screen (Blur)

```cpp
graph.add(CompositionNode(&sceneFBO, &blurFBO, &blurX));
graph.add(CompositionNode(&blurFBO, &sceneFBO, &blurY));
graph.add(CompositionNode(&sceneFBO, NULL, &composite));

graph.run(quad);
```

### C. Ping-Pong（FBO A <-> FBO B）

```cpp
for (int i = 0; i < iterations; ++i) {
    graph.add(CompositionNode(&fboA, &fboB, &step));
    graph.add(CompositionNode(&fboB, &fboA, &step));
}
graph.run(quad);
```

### D. Multi-Texture Composition (Glow / SSAO)

```cpp
CompositionNode node;
node.input  = &sceneFBO;
node.output = NULL;
node.shader = &composite;

node.addExtraTexture("uBlur", blurFBO.texture, 1);
node.addExtraTexture("uMask", maskFBO.texture, 2);

graph.add(node);
graph.run(quad);
```

## 4. Directory Structure

```text
OpenGL-cheat-sheet/
|- README.md
|- Composition.h
|- Composition.cpp
|- main.cpp
`- shaders/
   |- fullscreen.vert
   |- passthrough.frag
   |- blurX.frag
   |- blurY.frag
   `- composite.frag
```

## 5. License

MIT License

## 6. Planned Extensions

- MRT (Multiple Render Targets) support
- Compute Shader version (OpenGL 4.3+)
- Android / EGL / JNI version
- Multi-Target Graph for TextureView