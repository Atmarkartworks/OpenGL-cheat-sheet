#include "Composition.h"
#include <cstring> // memset

// ================= RenderTarget =================

RenderTarget::RenderTarget()
    : fbo(0), texture(0), depth(0), width(0), height(0) {
}

RenderTarget::~RenderTarget() {
    destroy();
}

bool RenderTarget::init(int w, int h) {
    width  = w;
    height = h;

    // Color Texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Depth Renderbuffer
    glGenRenderbuffers(1, &depth);
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                          width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // FBO
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER,
                              depth);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return (status == GL_FRAMEBUFFER_COMPLETE);
}

void RenderTarget::destroy() {
    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    if (texture) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
    if (depth) {
        glDeleteRenderbuffers(1, &depth);
        depth = 0;
    }
}

void RenderTarget::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
}

void RenderTarget::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ================= ShaderPass =================

ShaderPass::ShaderPass()
    : program(0) {
}

ShaderPass::~ShaderPass() {
    // program の破棄は外部ポリシーに任せるなら何もしない
    // glDeleteProgram(program);
}

void ShaderPass::use() const {
    glUseProgram(program);
}

void ShaderPass::setTexture(const char* name, GLuint tex, int unit) const {
    GLint loc = glGetUniformLocation(program, name);
    if (loc < 0) return;

    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(loc, unit);
}

void ShaderPass::setInt(const char* name, int v) const {
    GLint loc = glGetUniformLocation(program, name);
    if (loc < 0) return;
    glUniform1i(loc, v);
}

void ShaderPass::setFloat(const char* name, float v) const {
    GLint loc = glGetUniformLocation(program, name);
    if (loc < 0) return;
    glUniform1f(loc, v);
}

// ================= FullscreenQuad =================

FullscreenQuad::FullscreenQuad()
    : vao(0), vbo(0) {
}

FullscreenQuad::~FullscreenQuad() {
    destroy();
}

bool FullscreenQuad::init() {
    // 位置 + UV のフルスクリーンクアッド
    // (x, y, u, v)
    const float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vertices),
                 vertices,
                 GL_STATIC_DRAW);

    // layout(location = 0) vec2 aPos;
    // layout(location = 1) vec2 aUV;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float),
                          (const void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float),
                          (const void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

void FullscreenQuad::destroy() {
    if (vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}

void FullscreenQuad::draw() const {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// ================= CompositionNode =================

CompositionNode::CompositionNode()
    : input(0), output(0), shader(0), extraCount(0) {
    std::memset(extraTextures, 0, sizeof(extraTextures));
    std::memset(extraNames, 0, sizeof(extraNames));
    std::memset(extraUnits, 0, sizeof(extraUnits));
}

void CompositionNode::addExtraTexture(const char* name,
                                      GLuint tex,
                                      int unit) {
    if (extraCount >= MAX_EXTRA_TEXTURES) return;
    extraNames[extraCount]    = name;
    extraTextures[extraCount] = tex;
    extraUnits[extraCount]    = unit;
    ++extraCount;
}

void CompositionNode::execute(const FullscreenQuad& quad) const {
    if (!shader) return;

    if (output) {
        output->bind();
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // viewport は外側で設定している前提でもよい
    }

    shader->use();

    if (input) {
        shader->setTexture("uInput", input->texture, 0);
    }

    int i;
    for (i = 0; i < extraCount; ++i) {
        shader->setTexture(extraNames[i],
                           extraTextures[i],
                           extraUnits[i]);
    }

    // ポストエフェクト前提で深度テストはオフにしておくことが多い
    GLboolean depthEnabled = glIsEnabled(GL_DEPTH_TEST);
    if (depthEnabled) glDisable(GL_DEPTH_TEST);

    quad.draw();

    if (depthEnabled) glEnable(GL_DEPTH_TEST);

    if (output) {
        output->unbind();
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

// ================= CompositionGraph =================

void CompositionGraph::clear() {
    nodes.clear();
}

void CompositionGraph::add(const CompositionNode& node) {
    nodes.push_back(node);
}

void CompositionGraph::run(const FullscreenQuad& quad) const {
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].execute(quad);
    }
}
