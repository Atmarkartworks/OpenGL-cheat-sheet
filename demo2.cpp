#include "Composition.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdio>

static bool keyPressedEdge(GLFWwindow* win, int key, bool& wasDown) {
    const bool isDown = (glfwGetKey(win, key) == GLFW_PRESS);
    const bool pressed = isDown && !wasDown;
    wasDown = isDown;
    return pressed;
}

static void applyGaussianParams(ShaderPass& shGX,
                                ShaderPass& shGY,
                                int w,
                                int h,
                                int radius,
                                float sigma) {
    shGX.use();
    shGX.setFloat("texel", 1.0f / (float)w);
    shGX.setFloat("sigma", sigma);
    shGX.setInt("radius", radius);

    shGY.use();
    shGY.setFloat("texel", 1.0f / (float)h);
    shGY.setFloat("sigma", sigma);
    shGY.setInt("radius", radius);
}

static GLuint loadRawRGB(const char* path, int w, int h) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        std::cerr << "Failed to open: " << path << std::endl;
        return 0;
    }

    const size_t bytes = (size_t)(w * h * 3);
    unsigned char* data = new unsigned char[bytes];
    size_t readBytes = fread(data, 1, bytes, f);
    fclose(f);

    if (readBytes != bytes) {
        std::cerr << "Unexpected file size/content: " << path << std::endl;
        delete[] data;
        return 0;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] data;
    return tex;
}

int main() {
    const int W = 640;
    const int H = 480;

    if (!glfwInit()) {
        std::cerr << "glfwInit failed" << std::endl;
        return 1;
    }

    GLFWwindow* win = glfwCreateWindow(W, H, "Gaussian Blur Demo", NULL, NULL);
    if (!win) {
        std::cerr << "glfwCreateWindow failed" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(win);
    if (glewInit() != GLEW_OK) {
        std::cerr << "glewInit failed" << std::endl;
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }

    FullscreenQuad quad;
    quad.init();

    GLuint texBase = loadRawRGB("Atmarkartworks.bin", W, H);
    if (!texBase) {
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }

    RenderTarget ping;
    ping.init(W, H);

    ShaderPass shGX;
    ShaderPass shGY;
    shGX.program = createProgram("shaders/fullscreen.vert", "shaders/gaussianX.frag");
    shGY.program = createProgram("shaders/fullscreen.vert", "shaders/gaussianY.frag");

    if (!shGX.program || !shGY.program) {
        std::cerr << "Shader program creation failed" << std::endl;
        glDeleteTextures(1, &texBase);
        glfwDestroyWindow(win);
        glfwTerminate();
        return 1;
    }

    int radius = 10;
    float sigma = 6.0f;
    applyGaussianParams(shGX, shGY, W, H, radius, sigma);

    std::cout << "Controls: Up/Down=radius, Left/Right=sigma, R=reset, Esc=exit"
              << std::endl;
    std::cout << "Initial params: radius=" << radius
              << " sigma=" << sigma << std::endl;

    CompositionNode passX;
    passX.input = NULL;
    passX.output = &ping;
    passX.shader = &shGX;
    passX.addExtraTexture("uInput", texBase, 0);

    CompositionNode passY;
    passY.input = &ping;
    passY.output = NULL;
    passY.shader = &shGY;

    CompositionGraph graph;
    graph.add(passX);
    graph.add(passY);

    bool upWasDown = false;
    bool downWasDown = false;
    bool leftWasDown = false;
    bool rightWasDown = false;
    bool resetWasDown = false;

    while (!glfwWindowShouldClose(win)) {
        bool changed = false;

        if (keyPressedEdge(win, GLFW_KEY_UP, upWasDown)) {
            if (radius < 32) {
                ++radius;
                changed = true;
            }
        }
        if (keyPressedEdge(win, GLFW_KEY_DOWN, downWasDown)) {
            if (radius > 0) {
                --radius;
                changed = true;
            }
        }
        if (keyPressedEdge(win, GLFW_KEY_RIGHT, rightWasDown)) {
            sigma += 0.2f;
            changed = true;
        }
        if (keyPressedEdge(win, GLFW_KEY_LEFT, leftWasDown)) {
            if (sigma > 0.2f) {
                sigma -= 0.2f;
                changed = true;
            }
        }
        if (keyPressedEdge(win, GLFW_KEY_R, resetWasDown)) {
            radius = 10;
            sigma = 6.0f;
            changed = true;
        }

        if (changed) {
            applyGaussianParams(shGX, shGY, W, H, radius, sigma);
            std::cout << "radius=" << radius << " sigma=" << sigma << std::endl;
        }

        if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(win, GLFW_TRUE);
        }

        glViewport(0, 0, W, H);
        graph.run(quad);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteTextures(1, &texBase);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
