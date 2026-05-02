#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uInput;
uniform float texel;
uniform float sigma;
uniform int radius;

float gaussian(float x, float s) {
    return exp(-(x * x) / (2.0 * s * s));
}

void main() {
    vec3 sum = vec3(0.0);
    float wsum = 0.0;

    for (int i = -32; i <= 32; ++i) {
        if (abs(i) > radius) continue;
        float fi = float(i);
        float w = gaussian(fi, sigma);
        vec2 uv = vUV + vec2(fi * texel, 0.0);
        sum += texture(uInput, uv).rgb * w;
        wsum += w;
    }

    FragColor = vec4(sum / max(wsum, 1e-6), 1.0);
}
