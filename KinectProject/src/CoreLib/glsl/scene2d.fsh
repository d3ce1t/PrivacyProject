#version 130
varying vec2 v_texCoord;
uniform int stage;
uniform sampler2D texForeground;
uniform sampler2D texMask;
uniform sampler2D texBackground;

vec4 computeBG(float mask);

void main()
{
    float mask = texture2D(texMask, v_texCoord).r;

    // input: fgColor, mask, bgColor
    // Compute background (always)
    if (stage == 1) {
        gl_FragColor = computeBG(mask);
    }
    // input: fgColor
    // Render Background into FBO (fg or bg)
    else {
        gl_FragColor = texture2D(texForeground, v_texCoord);
    }
}

// Stage 1
vec4 computeBG(float mask)
{
    vec4 result;
    vec4 fgColor = texture2D(texForeground, v_texCoord);
    vec4 bgColor = texture2D(texBackground, v_texCoord); // stage 1: bgCopy

    if (mask > 0) {
        result = bgColor;
    } else {
        result = fgColor;
    }

    return result;
}
