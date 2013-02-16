#version 130

in vec4 posAttr;
in vec4 colAttr;

smooth out vec4 theColor;

void main()
{
    gl_Position = posAttr;
    theColor = colAttr;
}
