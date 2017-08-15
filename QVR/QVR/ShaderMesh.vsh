attribute vec2 a_tex_coord;
attribute vec4 a_pos;

varying vec2 v_tex_coord;

uniform mat4 u_mvpMat;

void main() {
    v_tex_coord = a_tex_coord;
    gl_Position = a_pos * u_mvpMat;
}
