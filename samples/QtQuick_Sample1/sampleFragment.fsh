#version 130

out vec4 outputColor;

//smooth in vec4 theColor;
uniform float fragLoopDuration;
uniform float time;

const vec4 firstColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
const vec4 secondColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);

void main()
{
    float currTime = mod(time, fragLoopDuration);
    float currLerp = currTime / fragLoopDuration;

    if (currTime < fragLoopDuration / 2)
        outputColor = mix(firstColor, secondColor, 2*currLerp);
    else
        outputColor = mix(firstColor, secondColor, 2 - 2*currLerp);
}


