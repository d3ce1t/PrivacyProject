#version 130
#extension GL_EXT_gpu_shader4 : enable    //Include support for this extension, which defines usampler2D
varying vec2 v_texCoord;
uniform sampler2D texSampler;

void main()
{
    /*float label = texture2D(texSampler, v_texCoord);

    if (label == 1) {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else if (label == 2) {
        gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
    } else {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }*/

    gl_FragColor = texture2D(texSampler, v_texCoord);
}
