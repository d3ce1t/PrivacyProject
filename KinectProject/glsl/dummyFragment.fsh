varying highp vec4 v_color;
varying highp vec2 v_textureCoord;
uniform sampler2D textureSampler;
uniform highp bool useTexture;
uniform int stage;

void main(void)
{
    if (stage == 1) {
        if (useTexture) {
            gl_FragColor = texture2D(textureSampler, v_textureCoord);
        }
        else {
            gl_FragColor = v_color;
        }
    } else {
        gl_FragColor = texture2D(textureSampler, v_textureCoord);
    }
}
