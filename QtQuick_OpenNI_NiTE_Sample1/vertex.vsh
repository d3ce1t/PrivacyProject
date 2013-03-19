#version 130

in vec4 posAttr;
in vec4 colAttr;
uniform float loopDuration;
uniform float time;

smooth out vec4 theColor;

void main()
{
    float timeScale = 3.14159f * 2.0f / loopDuration;
    float currTime = mod(time, loopDuration);
    vec4 totalOffset = vec4(cos(currTime * timeScale) * 0.25f,
                            sin(currTime * timeScale) * 0.25f,
                            0.0f,
                            0.0f);

    gl_Position = posAttr + totalOffset;
    theColor = colAttr;
}
