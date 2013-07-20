#version 130

in vec4 posAttr;
in vec2 texCoord;
uniform int stage = 1;
uniform mat4 perspectiveMatrix;
varying vec2 v_texCoord;

void main()
{
    vec4 position;

    if (stage == 1) {
        position = posAttr;
    } else {
        position = perspectiveMatrix * posAttr;
        position.y = -position.y;
    }

    gl_Position = position;
    v_texCoord = texCoord;
}
