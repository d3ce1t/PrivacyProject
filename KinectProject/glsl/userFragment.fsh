#version 130
varying vec2 v_texCoord;
// 0: no filter; 1: invisibility; 2: blur; 3: pixelation
// 4: emboss, 5: silhouette
uniform int currentFilter;

uniform int stage;
uniform sampler2D texForeground;
uniform sampler2D texMask;

vec4 blurEffectH();
vec4 blurEffectV();
vec4 pixelation();
vec4 embossEffect();

const vec2 textureSize = vec2(640, 480);
const float blurSizeH = 1.0/(0.5 * textureSize.x);
const float blurSizeV = 1.0/(0.5 * textureSize.y);

void main()
{
    float mask = texture2D(texMask, v_texCoord).r;

    if (mask == 0.0) {
        discard;
    }

    if (stage == 1)
    {
        // BlurH filter (first-pass) second-pass in textureFragment
        if (currentFilter == 2) {
            gl_FragColor = blurEffectH();
        }
        // Pixelation
        else if (currentFilter == 3) {
            gl_FragColor = pixelation();
        }
        // Emboss
        else if (currentFilter == 4) {
            gl_FragColor = embossEffect();
        }
        // Silhouette
        else if (currentFilter == 5) {
            gl_FragColor = vec4(0.8, 0.5, 0.0, 1.0);
        }
        // Nothing
        else {
            discard;
        }
    }
    else if (stage == 2)
    {
        if (currentFilter == 2) {
            gl_FragColor = blurEffectV();
        }
        else {
            gl_FragColor = texture2D(texForeground, v_texCoord);
        }
    }
}

vec4 blurEffectH()
{
    vec4 sum = vec4(0.0);

    // blur in y (vertical)
    // take nine samples, with the distance blurSize between them
    sum += texture2D(texForeground, vec2(v_texCoord.x - 4.0*blurSizeH, v_texCoord.y)) * 0.05;
    sum += texture2D(texForeground, vec2(v_texCoord.x - 3.0*blurSizeH, v_texCoord.y)) * 0.09;
    sum += texture2D(texForeground, vec2(v_texCoord.x - 2.0*blurSizeH, v_texCoord.y)) * 0.12;
    sum += texture2D(texForeground, vec2(v_texCoord.x - blurSizeH, v_texCoord.y)) * 0.15;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y)) * 0.16;
    sum += texture2D(texForeground, vec2(v_texCoord.x + blurSizeH, v_texCoord.y)) * 0.15;
    sum += texture2D(texForeground, vec2(v_texCoord.x + 2.0*blurSizeH, v_texCoord.y)) * 0.12;
    sum += texture2D(texForeground, vec2(v_texCoord.x + 3.0*blurSizeH, v_texCoord.y)) * 0.09;
    sum += texture2D(texForeground, vec2(v_texCoord.x + 4.0*blurSizeH, v_texCoord.y)) * 0.05;

    return sum;
}

vec4 blurEffectV()
{
    vec4 sum = vec4(0.0);

    // blur in y (vertical)
    // take nine samples, with the distance blurSize between them
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y - 4.0*blurSizeV)) * 0.05;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y - 3.0*blurSizeV)) * 0.09;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y - 2.0*blurSizeV)) * 0.12;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y - blurSizeV)) * 0.15;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y)) * 0.16;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y + blurSizeV)) * 0.15;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y + 2.0*blurSizeV)) * 0.12;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y + 3.0*blurSizeV)) * 0.09;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y + 4.0*blurSizeV)) * 0.05;

    return sum;
}

vec4 pixelation()
{
    vec2 dpos = 5 * (1/textureSize);
    vec2 coord = vec2(dpos.x * floor(v_texCoord.x / dpos.x), dpos.y * floor(v_texCoord.y / dpos.y));
    return texture2D(texForeground, coord);
}

vec4 embossEffect()
{
    vec2 onePixel = 1 / textureSize;
    vec4 color;
    color.rgb = vec3(0.5);
    color -= texture2D(texForeground, v_texCoord - onePixel) * 5.0;
    color += texture2D(texForeground, v_texCoord + onePixel) * 5.0;
    color.rgb = vec3((color.r + color.g + color.b) / 3.0);
    return vec4(color.rgb, 1);
}
