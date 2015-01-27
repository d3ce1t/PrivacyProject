#version 130
attribute vec4 posAttr;
attribute vec2 texCoord;
varying vec2 v_texCoord;

void main()
{
    gl_Position = posAttr;
    v_texCoord = texCoord;
}
