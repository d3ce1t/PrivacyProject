#version 130
varying vec2 v_texCoord;
uniform sampler2D texForeground;

void main()
{
    gl_FragColor = texture2D(texForeground, v_texCoord);
    //gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);
}
