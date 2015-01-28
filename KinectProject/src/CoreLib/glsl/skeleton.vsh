#version 130
attribute highp vec4 posAttr;
attribute highp vec4 colAttr;
uniform int units; // 0 - milimeters, 1 - meters, 2 - pixels
uniform vec2 frameSize;
uniform highp bool  mode3d = true;
uniform highp float pointSize;
uniform highp mat4 perspectiveMatrix;
varying highp vec4 theColor;

const vec2 fd = vec2(1.0 / 5.9421434211923247e+02, 1.0 / 5.9104053696870778e+02);

vec2 convertRealWorldToDepth(vec3 coordinates)
{
    vec2 cd = vec2(0.5 * frameSize.x, 0.5 * frameSize.y);
    return (coordinates.xy) / (abs(coordinates.z) * fd) + cd;
}

void main()
{
    vec3 position;

    // Convert to pixel coordinates (falta considerar si son metros o mm)
    if (!mode3d && units != 2) {
        position = vec3(convertRealWorldToDepth(posAttr.xyz), 0);
    }
    else if (units == 2) {
        position.xz = posAttr.xz;
        position.y = frameSize.y - posAttr.y; // Y goes from bottom to top in OpenGL
    }
    else {
        position = posAttr.xyz;
    }

    theColor = colAttr;
    gl_Position = perspectiveMatrix * vec4(position, 1.0);
    gl_PointSize = pointSize;
}
