#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uInput;
uniform sampler2D uBlur;

void main() {
    vec3 base = texture(uInput, vUV).rgb;
    vec3 glow = texture(uBlur, vUV).rgb;
    FragColor = vec4(base + glow, 1.0);
}
