#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uInput;

void main() {
    vec3 c = texture(uInput, vUV).rgb;
    // R channel is zeroed; only G and B are kept.
    FragColor = vec4(0.0, c.g, c.b, 1.0);
}
