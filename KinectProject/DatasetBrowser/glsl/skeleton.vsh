#version 130
attribute highp vec4 posAttr;
attribute highp vec4 colAttr;
uniform highp float pointSize;
uniform highp bool  mode3d = true;
uniform highp float width = 640;
uniform highp float height = 480;
uniform highp mat4 perspectiveMatrix;
varying highp vec4 theColor;

const vec2 fd = vec2(1.0 / 5.9421434211923247e+02, 1.0 / 5.9104053696870778e+02);

vec2 convertRealWorldToDepth(vec3 coordinates)
{
    vec2 cd = vec2(0.5 * width, 0.5 * height);
    return (coordinates.xy) / (abs(coordinates.z) * fd) + cd;
}

void main()
{
    vec3 position = mode3d ? posAttr.xyz : vec3(convertRealWorldToDepth(posAttr.xyz), 0);
    position.x = position.x;
    position.y = position.y;
    position.z -= 0.8; // Hack to show a little bit far from camera
    theColor = colAttr;
    gl_Position = perspectiveMatrix * vec4(position, 1.0);
    gl_PointSize = pointSize;
}
