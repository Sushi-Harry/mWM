#version 330
in vec2 texcoord;
uniform sampler2D tex;
uniform float opacity;

vec4 default_post_processing(vec4 c);

vec4 window_shader() {
    vec4 c = texelFetch(tex, ivec2(texcoord), 0);
    float lum = dot(c.rgb, vec3(0.2126, 0.7152, 0.0722));
    c.rgb = vec3(lum);
    return default_post_processing(c);
}