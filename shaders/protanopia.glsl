#version 330
in vec2 texcoord;
uniform sampler2D tex;
uniform float opacity;

vec4 default_post_processing(vec4 c);

vec4 window_shader() {
    vec4 c = texelFetch(tex, ivec2(texcoord), 0);

    // RGB to LMS cone response
    float L = 0.31399022 * c.r + 0.63951294 * c.g + 0.04649755 * c.b;
    float M = 0.15537241 * c.r + 0.75789446 * c.g + 0.08670142 * c.b;
    float S = 0.01775239 * c.r + 0.10944209 * c.g + 0.87256922 * c.b;

    // Simulate Protanopia (missing L-cones)
    float sim_L = 1.05118294 * M - 0.05116099 * S;

    // LMS back to RGB
    vec3 sim_rgb = vec3(
         5.47221206 * sim_L - 4.64196010 * M + 0.16963708 * S,
        -1.12524190 * sim_L + 2.29317094 * M - 0.16789520 * S,
         0.02980165 * sim_L - 0.19318073 * M + 1.16364789 * S
    );

    // Shift lost red contrast into green and blue channels
    vec3 err = c.rgb - sim_rgb;
    vec3 correction = vec3(0.0, 0.7 * err.r, 0.7 * err.r);
    c.rgb = clamp(c.rgb + correction, 0.0, 1.0);

    return default_post_processing(c);
}