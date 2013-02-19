#version 130

in vec4 posAttr;
in vec4 colAttr;
uniform vec2 offset;

smooth out vec4 theColor;

void main()
{
    gl_Position = posAttr + vec4(offset.x, offset.y, 0.0, 0.0);
    theColor = colAttr;
}
