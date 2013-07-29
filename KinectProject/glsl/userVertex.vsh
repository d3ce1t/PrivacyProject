#version 130

in vec4 posAttr;
in vec2 texCoord;
uniform mat4 perspectiveMatrix;
varying vec2 v_texCoord;

void main()
{
    vec4 position = posAttr;
    position.z -= 2.5;
    gl_Position = perspectiveMatrix * position;
    v_texCoord = texCoord;
}
