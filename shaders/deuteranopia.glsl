#version 330
in vec2 texcoord;
uniform sampler2D tex;
uniform float opacity;

vec4 default_post_processing(vec4 c);

vec4 window_shader() {
    vec4 c = texelFetch(tex, ivec2(texcoord), 0);

    float L = 0.31399022 * c.r + 0.63951294 * c.g + 0.04649755 * c.b;
    float M = 0.15537241 * c.r + 0.75789446 * c.g + 0.08670142 * c.b;
    float S = 0.01775239 * c.r + 0.10944209 * c.g + 0.87256922 * c.b;

    // Simulate Deuteranopia (missing M-cones)
    float sim_M = 0.95130920 * L + 0.04866992 * S;

    vec3 sim_rgb = vec3(
         5.47221206 * L - 4.64196010 * sim_M + 0.16963708 * S,
        -1.12524190 * L + 2.29317094 * sim_M - 0.16789520 * S,
         0.02980165 * L - 0.19318073 * sim_M + 1.16364789 * S
    );

    // Shift lost green contrast into red and blue channels
    vec3 err = c.rgb - sim_rgb;
    vec3 correction = vec3(0.7 * err.g, 0.0, 0.7 * err.g);
    c.rgb = clamp(c.rgb + correction, 0.0, 1.0);

    return default_post_processing(c);
}