attribute highp vec4 posAttr;
attribute highp vec4 colAttr;
attribute highp float  maskAttr;
uniform highp float sizeAttr;
uniform highp mat4 perspectiveMatrix;
varying highp vec4 theColor;

void main()
{
    gl_PointSize = sizeAttr;
    gl_Position = perspectiveMatrix * posAttr;

    if (maskAttr == 0.0f) {
        theColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else if (maskAttr == 1.0f) {
        theColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else if (maskAttr == 2.0f) {
        theColor = vec4(0.0, 0.0, 1.0, 1.0);
    } else if (maskAttr == 3.0f) {
        theColor = vec4(1.0, 1.0, 0.0, 1.0);
    } else if (maskAttr == 4.0f) {
        theColor = vec4(0.0, 1.0, 1.0, 1.0);
    } else {
        theColor = colAttr;
    }
}
