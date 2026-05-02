#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uInput; // blurred image
uniform sampler2D uBlur;  // GB-only color-shifted image

void main() {
    vec3 blurred = texture(uInput, vUV).rgb;
    vec3 gbOnly  = texture(uBlur,  vUV).rgb;
    // Blend: blur base at 60% + GB-only overlay at 80% (additive tint)
    FragColor = vec4(blurred * 0.6 + gbOnly * 0.8, 1.0);
}
