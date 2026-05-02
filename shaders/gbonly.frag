#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uInput;

void main() {
    vec3 c = texture(uInput, vUV).rgb;
    // Source intensity mask (0..1). Black background becomes 0.
    float srcMask = max(c.r, max(c.g, c.b));

    // Optional hard threshold using step().
    // threshold: 0.0 = effectively no cutoff, larger value clips darker pixels.
    // useStep:   0.0 = disabled (default/no visual change), 1.0 = enabled.
    // Example: threshold=0.2 and useStep=1.0 for binary-like text masking.
    float threshold = 0.0;
    float useStep = 0.0;
    float hardMask = step(threshold, srcMask);

    // Default path keeps original smooth mask; step() can be blended in as a parameter.
    float mask = mix(srcMask, hardMask, useStep);

    FragColor = vec4(0.0, c.g, c.b, mask);
}
