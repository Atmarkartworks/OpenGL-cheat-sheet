#ifndef COMPOSITION_H
#define COMPOSITION_H

// Adjust includes for your environment if needed.
#include <GL/glew.h>
// e.g. #include <GLES3/gl3.h>

#include <vector>

// Simple RenderTarget: FBO + Color Texture + Depth Renderbuffer
struct RenderTarget {
    GLuint fbo;
    GLuint texture;
    GLuint depth;
    int width;
    int height;

    RenderTarget();
    ~RenderTarget();

    // Create a standard RGBA8 + Depth24 FBO.
    bool init(int w, int h);

    void destroy();

    void bind();
    void unbind();
};

// Simple ShaderPass: Program + texture binding helpers.
struct ShaderPass {
    GLuint program;

    ShaderPass();
    ~ShaderPass();

    void use() const;

    // Bind a texture to a sampler2D uniform.
    void setTexture(const char* name, GLuint tex, int unit) const;

    // Minimal scalar uniform helpers.
    void setInt(const char* name, int v) const;
    void setFloat(const char* name, float v) const;
};

// Utility for drawing a fullscreen quad.
struct FullscreenQuad {
    GLuint vao;
    GLuint vbo;

    FullscreenQuad();
    ~FullscreenQuad();

    bool init();
    void destroy();

    void draw() const;
};

// One pass: process input -> output with a shader.
struct CompositionNode {
    RenderTarget* input;   // NULL is allowed (e.g., when reading from screen).
    RenderTarget* output;  // NULL = default framebuffer
    ShaderPass*   shader;

    // Simple slots for passing additional textures.
    enum { MAX_EXTRA_TEXTURES = 4 };
    GLuint extraTextures[MAX_EXTRA_TEXTURES];
    const char* extraNames[MAX_EXTRA_TEXTURES];
    int extraUnits[MAX_EXTRA_TEXTURES];
    int extraCount;

    CompositionNode();

    void addExtraTexture(const char* name, GLuint tex, int unit);

    void execute(const FullscreenQuad& quad) const;
};

// Simple graph that executes nodes in sequence.
struct CompositionGraph {
    std::vector<CompositionNode> nodes;

    void clear();
    void add(const CompositionNode& node);
    void run(const FullscreenQuad& quad) const;
};

// Build a GL program from vertex/fragment shader source files.
GLuint createProgram(const char* vertexPath, const char* fragmentPath);

#endif // COMPOSITION_H
