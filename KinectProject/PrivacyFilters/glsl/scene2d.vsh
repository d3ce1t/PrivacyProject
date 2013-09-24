#version 130
attribute highp vec4 posAttr;
attribute highp vec2 texCoord;
uniform int stage;
uniform mat4 perspectiveMatrix;
varying vec2 v_texCoord;

void main()
{
    vec4 position = perspectiveMatrix * posAttr;

    if (stage == 3) {
        position.y = -position.y;
    }

    gl_Position = position;
    v_texCoord = texCoord;
}
