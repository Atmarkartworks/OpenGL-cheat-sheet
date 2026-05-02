#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uInput;
uniform float texel;

void main() {
    vec3 c =
        texture(uInput, vUV + vec2(0, -4.0*texel)).rgb * 0.05 +
        texture(uInput, vUV + vec2(0, -2.0*texel)).rgb * 0.25 +
        texture(uInput, vUV).rgb                     * 0.40 +
        texture(uInput, vUV + vec2(0,  2.0*texel)).rgb * 0.25 +
        texture(uInput, vUV + vec2(0,  4.0*texel)).rgb * 0.05;

    FragColor = vec4(c, 1.0);
}
