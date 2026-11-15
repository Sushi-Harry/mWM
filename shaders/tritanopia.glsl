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

    // Simulate Tritanopia (missing S-cones)
    float sim_S = -0.86744736 * L + 1.86727089 * M;

    vec3 sim_rgb = vec3(
         5.47221206 * L - 4.64196010 * M + 0.16963708 * sim_S,
        -1.12524190 * L + 2.29317094 * M - 0.16789520 * sim_S,
         0.02980165 * L - 0.19318073 * M + 1.16364789 * sim_S
    );

    // Shift lost blue contrast into red and green channels
    vec3 err = c.rgb - sim_rgb;
    vec3 correction = vec3(0.7 * err.b, 0.7 * err.b, 0.0);
    c.rgb = clamp(c.rgb + correction, 0.0, 1.0);

    return default_post_processing(c);
}