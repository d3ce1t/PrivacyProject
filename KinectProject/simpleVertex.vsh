attribute highp vec4 posAttr;
attribute highp vec4 colAttr;
uniform highp float sizeAttr;
uniform highp mat4 perspectiveMatrix;
varying highp vec4 theColor;

void main()
{
    gl_PointSize = sizeAttr;
    gl_Position = perspectiveMatrix * posAttr;
    theColor = colAttr;
}
