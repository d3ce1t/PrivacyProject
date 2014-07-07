#version 130
varying vec2 v_texCoord;
uniform sampler2D texColorFrame;

void main()
{
    vec4 pixel = texture2D(texColorFrame, v_texCoord);
    gl_FragColor = pixel;
}
