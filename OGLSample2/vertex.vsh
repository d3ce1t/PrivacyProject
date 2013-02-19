#version 130

in vec4 posAttr;
in vec4 colAttr;
uniform vec2 offset;
uniform float zNear;
uniform float zFar;
uniform float frustumScale;

smooth out vec4 theColor;

void main()
{
    vec4 cameraPos = posAttr + vec4(offset.x, offset.y, 0.0, 0.0);
    vec4 clipPos;

    clipPos.xy = cameraPos.xy * frustumScale;

    clipPos.z = cameraPos.z * (zNear + zFar) / (zNear - zFar);
    clipPos.z += 2 * zNear * zFar / (zNear - zFar);

    clipPos.w = -cameraPos.z;

    gl_Position = clipPos;
    theColor = colAttr;
}
