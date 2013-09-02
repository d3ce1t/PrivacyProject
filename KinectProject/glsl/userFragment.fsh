#version 130
varying vec2 v_texCoord;
uniform sampler2D texSampler;

void main()
{
    vec4 color = texture2D(texSampler, v_texCoord);

    if (color.r == 0.0 && color.g == 0.0 && color.b == 0.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        gl_FragColor = color;
    }
}
