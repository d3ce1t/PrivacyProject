#version 130
varying vec2 v_texCoord;
uniform int currentFilter;
uniform int stage;
uniform sampler2D texColor;
uniform sampler2D texMask;
uniform sampler2D texBackground;


void main()
{
    vec4 fgColor = texture2D(texColor, v_texCoord);
    float mask = texture2D(texMask, v_texCoord).r;
    vec4 bgColor = texture2D(texBackground, v_texCoord); // stage 1: bgCopy; stage 2: fbo

    if (stage == 1)
    {
        if (mask > 0) {
            gl_FragColor = bgColor;
        } else {
            gl_FragColor = fgColor;
        }
    }
    else {
        if (currentFilter != 0)
            gl_FragColor = bgColor;
        else
            gl_FragColor = fgColor;
    }
}
