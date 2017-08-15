attribute vec2 TextureCoords;
attribute vec4 Position;
varying vec2 TextureCoordsOut;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 p;

void main(void)
{
    vec4 pos = modelViewMatrix * Position;
    gl_Position = projectionMatrix * pos;
    TextureCoordsOut = TextureCoords;
}
