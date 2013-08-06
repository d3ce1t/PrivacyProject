#version 130
attribute highp vec2 posAttr;
attribute highp float distanceAttr;
uniform highp float width;
uniform highp float height;
uniform highp mat4 perspectiveMatrix;
varying highp float v_distance;

const vec2 fd = vec2(1.0 / 5.9421434211923247e+02, 1.0 / 5.9104053696870778e+02);

vec3 convertDepthToRealWorld(vec2 coordinates, float distance)
{
    vec2 cd = vec2(0.5 * width, 0.5 * height);
    vec3 result;
    result.xy = (coordinates - cd) * distance * fd;
    result.z = -distance;
    result.y = -result.y;
    return result;
}

void main()
{
    vec3 realPos = convertDepthToRealWorld(posAttr, distanceAttr);
    gl_Position = perspectiveMatrix * vec4(realPos.xyz, 1.0);
    v_distance = distanceAttr;
}


