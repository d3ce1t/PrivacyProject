#version 130

in vec4 posAttr;
in vec4 colAttr;
uniform vec2 offset;

smooth out vec4 theColor;

void main()
{
    vec4 totalOffset = vec4(offset.x, offset.y, 0.0, 0.0);
    gl_Position = posAttr + totalOffset;
    theColor = colAttr;
}
