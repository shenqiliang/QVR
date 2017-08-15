attribute vec2 TextureCoords;
attribute vec4 Position;
varying vec2 TextureCoordsOut;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 p;

void main(void)
{
    vec4 pos = modelViewMatrix * Position;
    if (pos.z >= 0.0) {
        float r2 = dot(pos.xy, pos.xy) / (300.0*300.0*2.0);
        pos.xy *= 1.0 + p[0]*r2 + p[1]*r2*r2 + p[2]*r2*r2*r2;
    }
    gl_Position = projectionMatrix * pos;
    TextureCoordsOut = TextureCoords;
}
