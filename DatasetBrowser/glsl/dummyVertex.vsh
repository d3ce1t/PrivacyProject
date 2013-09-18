attribute highp vec4 posAttr;
attribute highp vec4 colorAttr;
attribute highp vec2 textureCoordAttr;
uniform highp vec2 posOffset;
uniform highp bool useTexture;
uniform int stage;
varying highp vec4 v_color;
varying highp vec2 v_textureCoord;

//uniform mat4 perspectiveMatrix;

void main(void)
{
    gl_Position = vec4(posOffset.x, posOffset.y, 0, 0) + posAttr;
    v_color = colorAttr;
    v_textureCoord = textureCoordAttr;
}
