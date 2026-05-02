#include "Composition.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdio>

static GLuint loadRawRGB(const char* path, int w, int h) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        std::cerr << "Failed to open: " << path << std::endl;
        return 0;
    }
    unsigned char* data = new unsigned char[w * h * 3];
    fread(data, 1, (size_t)(w * h * 3), f);
    fclose(f);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] data;
    return tex;
}

int main() {
    const int W = 640, H = 480;

    glfwInit();
    GLFWwindow* win = glfwCreateWindow(W, H, "Blur + GB Composite Demo", NULL, NULL);
    glfwMakeContextCurrent(win);
    glewInit();

    FullscreenQuad quad;
    quad.init();

    // Load 640x480 RGB raw image as OpenGL texture.
    GLuint texBase = loadRawRGB("Atmarkartworks.bin", W, H);

    // Intermediate FBOs.
    RenderTarget blurFBO_x; // after horizontal blur
    RenderTarget blurFBO;   // after vertical blur   (final blur result)
    RenderTarget gbFBO;     // GB-only color-shifted image
    blurFBO_x.init(W, H);
    blurFBO.init(W, H);
    gbFBO.init(W, H);

    // Shader programs.
    ShaderPass shBlurX, shBlurY, shGB, shComposite;
    shBlurX.program     = createProgram("shaders/fullscreen.vert", "shaders/blurX.frag");
    shBlurY.program     = createProgram("shaders/fullscreen.vert", "shaders/blurY.frag");
    shGB.program        = createProgram("shaders/fullscreen.vert", "shaders/gbonly.frag");
    shComposite.program = createProgram("shaders/fullscreen.vert", "shaders/composite-demo.frag");

    // Pre-set texel size uniforms (values persist in the program object).
    shBlurX.use(); shBlurX.setFloat("texel", 1.0f / (float)W);
    shBlurY.use(); shBlurY.setFloat("texel", 1.0f / (float)H);

    // ----- Build composition graph -----

    // Pass 1: BlurX  texBase -> blurFBO_x
    // texBase is a raw GL texture (not a RenderTarget), so pass via extraTextures.
    CompositionNode passBlurX;
    passBlurX.input  = NULL;
    passBlurX.output = &blurFBO_x;
    passBlurX.shader = &shBlurX;
    passBlurX.addExtraTexture("uInput", texBase, 0);

    // Pass 2: BlurY  blurFBO_x -> blurFBO
    CompositionNode passBlurY;
    passBlurY.input  = &blurFBO_x;
    passBlurY.output = &blurFBO;
    passBlurY.shader = &shBlurY;

    // Pass 3: GB-only  texBase -> gbFBO
    CompositionNode passGB;
    passGB.input  = NULL;
    passGB.output = &gbFBO;
    passGB.shader = &shGB;
    passGB.addExtraTexture("uInput", texBase, 0);

    // Pass 4: Composite  blurFBO (uInput) + gbFBO (uBlur) -> screen
    CompositionNode passComp;
    passComp.input  = &blurFBO;
    passComp.output = NULL;       // screen
    passComp.shader = &shComposite;
    passComp.addExtraTexture("uBlur", gbFBO.texture, 1);

    CompositionGraph graph;
    graph.add(passBlurX);
    graph.add(passBlurY);
    graph.add(passGB);
    graph.add(passComp);

    while (!glfwWindowShouldClose(win)) {
        glViewport(0, 0, W, H);
        graph.run(quad);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteTextures(1, &texBase);
    return 0;
}
