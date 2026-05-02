#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uInput; // blurred image
uniform sampler2D uBlur;  // GB-only color-shifted image

void main() {
    vec3 blurred = texture(uInput, vUV).rgb;
    vec4 gb      = texture(uBlur,  vUV);

    // Overlay GB tint only where source is non-black (mask is stored in gb.a).
    vec3 color = blurred + gb.rgb * gb.a;
    FragColor = vec4(color, 1.0);
}
