#ifndef COMPOSITION_H
#define COMPOSITION_H

// 必要に応じて環境に合わせて変更
#include <GL/glew.h>
// #include <GLES3/gl3.h> など

#include <vector>

// シンプルな RenderTarget: FBO + Color Texture + Depth Renderbuffer
struct RenderTarget {
    GLuint fbo;
    GLuint texture;
    GLuint depth;
    int width;
    int height;

    RenderTarget();
    ~RenderTarget();

    // RGBA8 + Depth24 の標準的な FBO を生成
    bool init(int w, int h);

    void destroy();

    void bind();
    void unbind();
};

// シンプルな ShaderPass: Program + テクスチャバインドヘルパ
struct ShaderPass {
    GLuint program;

    ShaderPass();
    ~ShaderPass();

    void use() const;

    // sampler2D uniform にテクスチャをバインド
    void setTexture(const char* name, GLuint tex, int unit) const;

    // int / float など最低限
    void setInt(const char* name, int v) const;
    void setFloat(const char* name, float v) const;
};

// フルスクリーンクアッド描画用ユーティリティ
struct FullscreenQuad {
    GLuint vao;
    GLuint vbo;

    FullscreenQuad();
    ~FullscreenQuad();

    bool init();
    void destroy();

    void draw() const;
};

// 1 パス分: input → output を shader で処理
struct CompositionNode {
    RenderTarget* input;   // NULL も許容（例: 画面から読む場合など）
    RenderTarget* output;  // NULL = default framebuffer
    ShaderPass*   shader;

    // 追加で複数テクスチャを渡したい場合用の簡易スロット
    enum { MAX_EXTRA_TEXTURES = 4 };
    GLuint extraTextures[MAX_EXTRA_TEXTURES];
    const char* extraNames[MAX_EXTRA_TEXTURES];
    int extraUnits[MAX_EXTRA_TEXTURES];
    int extraCount;

    CompositionNode();

    void addExtraTexture(const char* name, GLuint tex, int unit);

    void execute(const FullscreenQuad& quad) const;
};

// ノード列を順に実行するだけのシンプルなグラフ
struct CompositionGraph {
    std::vector<CompositionNode> nodes;

    void clear();
    void add(const CompositionNode& node);
    void run(const FullscreenQuad& quad) const;
};

#endif // COMPOSITION_H
