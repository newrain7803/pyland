#version 130

in vec2 f_texture_coord;
uniform sampler2D texture;
uniform vec4 colour;

void main() {
    float alpha = texture2D(texture, f_texture_coord).a;
    
    if (alpha == 0.0) {
        discard;
    }

    gl_FragColor.rgb   = colour.rgb;
    gl_FragColor.a   = colour.a * alpha;
}
