#version 130
in vec4 posAttr;
in vec2 texCoord;
uniform int stage;
varying vec2 v_texCoord;

void main()
{
    gl_Position = posAttr;
    v_texCoord = texCoord;
}
