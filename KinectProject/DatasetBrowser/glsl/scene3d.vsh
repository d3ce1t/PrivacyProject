#version 130
attribute highp float indexAttr;
attribute highp float distanceAttr;
uniform highp float width = 320;
uniform highp float height = 240;
uniform highp mat4 perspectiveMatrix;
varying highp float v_distance;

const vec2 fd = vec2(1.0 / 5.9421434211923247e+02, 1.0 / 5.9104053696870778e+02);

vec3 convertDepthToRealWorld(vec2 coordinates, float distance)
{
    vec2 cd = vec2(0.5 * width, 0.5 * height);
    vec3 result;
    result.xy = (coordinates - cd) * distance * fd;
    result.xy = 2 * result.xy; // becaise I want to scale XY to 640x480
    result.z = -distance;
    result.y = -result.y;
    return result;
}

void main()
{
    float row = floor(indexAttr / width);
    float col = mod(indexAttr, width);
    vec3 realPos = convertDepthToRealWorld(vec2(col, row), distanceAttr);
    realPos.z -= 0.8; // Hack to show a little bit far from camera
    gl_Position = perspectiveMatrix * vec4(realPos.xyz, 1.0);
    v_distance = distanceAttr;
}


