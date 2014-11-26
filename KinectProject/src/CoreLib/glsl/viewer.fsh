#version 130
varying vec2 v_texCoord;
uniform sampler2D texColorFrame;
uniform bool notexture;

void main()
{    
    if (notexture) {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        gl_FragColor = texture2D(texColorFrame, v_texCoord);
    }
}
