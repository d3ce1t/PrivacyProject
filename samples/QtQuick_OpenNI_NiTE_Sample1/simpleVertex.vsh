#version 130

in vec4 posAttr;
in vec4 colAttr;
uniform float sizeAttr = 1;
uniform mat4 perspectiveMatrix;

smooth out vec4 theColor;

void main()
{
    gl_PointSize = sizeAttr;
    gl_Position = perspectiveMatrix * posAttr;
    theColor = colAttr;
}
