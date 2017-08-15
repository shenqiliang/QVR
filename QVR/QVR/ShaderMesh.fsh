precision mediump float;

varying vec2 v_tex_coord;

uniform sampler2D u_texture;

void main(){
    gl_FragColor = texture2D(u_texture, v_tex_coord);
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
