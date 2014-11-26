#version 130
varying vec2 v_texCoord;
uniform sampler2D texBackground;

void main()
{
    gl_FragColor = texture2D(texBackground, v_texCoord);
}
