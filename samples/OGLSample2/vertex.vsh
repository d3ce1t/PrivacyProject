#version 130

in vec4 posAttr;
in vec4 colAttr;
uniform vec2 offset;
uniform mat4 perspectiveMatrix;


smooth out vec4 theColor;

void main()
{
    vec4 cameraPos = posAttr + vec4(offset.x, offset.y, 0.0, 0.0);
    gl_Position = perspectiveMatrix * cameraPos;
    theColor = colAttr;
}
