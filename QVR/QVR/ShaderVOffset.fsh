precision mediump float;

uniform sampler2D Texture;
varying vec2 TextureCoordsOut;
uniform int pixelFormat;

void main(void)
{
    vec4 mask = texture2D(Texture, TextureCoordsOut);
    if (TextureCoordsOut.x < 0.0) {
        gl_FragColor = vec4(0.0);
    }
    else if (pixelFormat == 1) {
        gl_FragColor = vec4(mask.bgr, 1.0);
    }
    else{
        gl_FragColor = vec4(mask.rgb, 1.0);
    }
}
