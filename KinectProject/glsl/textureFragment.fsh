#version 130
varying vec2 v_texCoord;
uniform sampler2D texColor;
uniform sampler2D texMask;
uniform sampler2D texBackground;

void main()
{
    float mask = texture2D(texMask, v_texCoord).r;
    vec4 fgColor = texture2D(texColor, v_texCoord);
    vec4 bgColor = texture2D(texBackground, v_texCoord);

    if (mask == 1.0) {
        gl_FragColor = bgColor;
    } else {
        gl_FragColor = fgColor;
    }
}
