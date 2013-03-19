#version 130

in vec4 posAttr;
in vec2 texCoord;
uniform mat4 perspectiveMatrix;
varying vec2 v_texCoord;

void main()
{
    gl_Position = perspectiveMatrix * posAttr;
    v_texCoord = texCoord;
}
