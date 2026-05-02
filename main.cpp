#include "Composition.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    glfwInit();
    GLFWwindow* win = glfwCreateWindow(800, 600, "Composition Demo", NULL, NULL);
    glfwMakeContextCurrent(win);
    glewInit();

    FullscreenQuad quad;
    quad.init();

    RenderTarget sceneFBO;
    sceneFBO.init(800, 600);

    ShaderPass passthrough;
    passthrough.program = createProgram("shaders/fullscreen.vert", "shaders/passthrough.frag");

    CompositionGraph graph;

    CompositionNode node;
    node.input  = &sceneFBO;
    node.output = NULL;
    node.shader = &passthrough;

    graph.add(node);

    while (!glfwWindowShouldClose(win)) {
        // 1. sceneFBO に描画
        sceneFBO.bind();
        glClearColor(0.2, 0.3, 0.4, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        sceneFBO.unbind();

        // 2. CompositionGraph 実行
        graph.run(quad);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    return 0;
}