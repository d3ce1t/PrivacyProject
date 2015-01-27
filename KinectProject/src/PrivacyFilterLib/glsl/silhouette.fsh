#version 130
varying vec2 v_texCoord;
uniform int silhouetteEffect; // 0: normal, 1: blur, 2: pixelation, 3_ emboss

uniform int stage;
uniform vec2 textureSize;
uniform sampler2D texForeground;
uniform sampler2D texMask;

vec4 blurEffectH();
vec4 blurEffectV();
vec4 pixelation();
vec4 embossEffect();

const float pixels_radio = 9;
const float blur_weight = 1 / (pixels_radio*2+1);

void main()
{
    float mask = texture2D(texMask, v_texCoord).r;

    if (mask == 0.0 || mask == 1.0)
        discard;

    if (stage == 1)
    {
        // Silhouette
        if (silhouetteEffect == 0) {
            gl_FragColor = vec4(0.5, 0.8, 0.0, 1.0);
        }
        // BlurH filter (first-pass) second-pass in textureFragment
        else if (silhouetteEffect == 1) {
            gl_FragColor = blurEffectH();
        }
        // Pixelation
        else if (silhouetteEffect == 2) {
            gl_FragColor = pixelation();
        }
        // Emboss
        else if (silhouetteEffect == 3) {
            gl_FragColor = embossEffect();
        }
        // Nothing
        else {
            discard;
        }
    }
    else if (stage == 2)
    {
        if (silhouetteEffect == 1) {
            gl_FragColor = blurEffectV();
        }
        else {
            gl_FragColor = texture2D(texForeground, v_texCoord);
        }
    }
}

vec4 blurEffectH()
{
    vec2 pixelSize = 1/textureSize;
    vec4 sum = vec4(0.0);

    for (float i=-pixels_radio; i<=pixels_radio; ++i) {
        sum += texture2D(texForeground, vec2(v_texCoord.x + i*pixelSize.x, v_texCoord.y)) * blur_weight;
    }

    return sum;
}

vec4 blurEffectV()
{
    vec2 pixelSize = 1/textureSize;
    vec4 sum = vec4(0.0);

    for (float i=-pixels_radio; i<=pixels_radio; ++i) {
        sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y + i*pixelSize.y)) * blur_weight;
    }

    return sum;
}

vec4 pixelation()
{
    vec2 pixelSize = 1/textureSize;
    vec2 dpos = 2*pixels_radio * pixelSize;
    vec2 coord = vec2(dpos.x * floor(v_texCoord.x / dpos.x), dpos.y * floor(v_texCoord.y / dpos.y));
    return texture2D(texForeground, coord);
}

vec4 embossEffect()
{
    vec2 pixelSize = 1/textureSize;
    vec4 color;
    color.rgb = vec3(0.5);
    color -= texture2D(texForeground, v_texCoord - pixelSize) * 5.0;
    color += texture2D(texForeground, v_texCoord + pixelSize) * 5.0;
    color.rgb = vec3((color.r + color.g + color.b) / 3.0);
    return vec4(color.rgb, 1);
}
