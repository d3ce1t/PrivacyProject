#version 130

in vec4 posAttr;
in vec2 texCoord;
uniform int stage;
uniform mat4 perspectiveMatrix;
uniform int currentFilter; // 0: no filter; 1: invisibility; 2: blur
varying vec2 v_texCoord;

void main()
{
    vec4 position = posAttr;

    if (stage == 1) {
        //position = posAttr;
    }
    else if (stage == 2) {
        //position = posAttr;
    }
    else {
        position.z -= 2.5;
        position = perspectiveMatrix * position;
        position.y = -position.y;
    }

    gl_Position = position;
    v_texCoord = texCoord;
}
