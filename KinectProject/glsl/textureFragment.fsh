#version 130
varying vec2 v_texCoord;
uniform int currentFilter;
uniform int stage;
uniform sampler2D texForeground;
uniform sampler2D texMask;
uniform sampler2D texBackground;

void main()
{
    if (stage == 1)
    {
        vec4 fgColor = texture2D(texForeground, v_texCoord);
        float mask = texture2D(texMask, v_texCoord).r;
        vec4 bgColor = texture2D(texBackground, v_texCoord); // stage 1: bgCopy

        if (mask > 0) {
            gl_FragColor = bgColor;
        } else {
            gl_FragColor = fgColor;
        }
    }
    else
    {
        vec4 fgColor = texture2D(texForeground, v_texCoord);
        vec4 bgColor = texture2D(texBackground, v_texCoord); // stage 2: fbo

        if (currentFilter != 0)
            gl_FragColor = bgColor;
        else
            gl_FragColor = fgColor;
    }
}
