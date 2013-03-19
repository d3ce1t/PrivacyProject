#version 130
varying vec2 v_texCoord;
uniform sampler2D texSampler;

void main()
{
    gl_FragColor = texture2D(texSampler, v_texCoord);
}
