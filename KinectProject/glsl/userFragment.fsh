#version 130
#extension GL_EXT_gpu_shader4 : enable    //Include support for this extension, which defines usampler2D
varying vec2 v_texCoord;
uniform sampler2D texSampler;

void main()
{
    gl_FragColor = texture2D(texSampler, v_texCoord);
}
