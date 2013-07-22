#version 130
varying vec2 v_texCoord;
uniform int currentFilter; // 0: no filter; 1: invisibility; 2: blur; 3: pixelation, 4: emboss
uniform int stage;
uniform sampler2D texForeground;
uniform sampler2D texMask;
uniform sampler2D texBackground;

vec4 computeBG(float mask);
vec4 embossEffect(float mask);
vec4 blurEffectH(float mask);
vec4 blurEffectV(float mask);
vec4 pixelation(float mask);

const vec2 textureSize = vec2(640, 480);
const float blurSizeH = 1.0/(0.5 * textureSize.x);
const float blurSizeV = 1.0/(0.5 * textureSize.y);

void main()
{
    float mask = texture2D(texMask, v_texCoord).r;
    vec4 fgColor = texture2D(texForeground, v_texCoord);
    vec4 bgColor = texture2D(texBackground, v_texCoord); // stage 1: bg, stage 2: fbo

    // input: fgColor, mask, bgColor
    // Compute background (always)
    if (stage == 1) {
        gl_FragColor = computeBG(mask);
    }
    // input: fgColor, mask, bgColor
    // Compute filter: first-past
    else if (stage == 2) {
        if (currentFilter == 0) { // No filter
            gl_FragColor = fgColor;
        }
        else if (currentFilter == 1) { // Invisibility (bg pass-through)
            gl_FragColor = bgColor;
        }
        else if (currentFilter == 2) { // BlurH filter (first-pass)
            gl_FragColor = blurEffectH(mask);
        }
        else if (currentFilter == 3) {
            gl_FragColor = pixelation(mask);
        }
        else if (currentFilter == 4) {
            gl_FragColor = embossEffect(mask);
        }
    }
    // input: mask, bgColor
    // Compute filter: second-pass
    else {
        if (currentFilter == 2) { // BlurV filter (second-pass)
            gl_FragColor = blurEffectV(mask);
        } else {
            gl_FragColor = fgColor; // pass-through result of stage 2
        }
    }
}

// Stage 1
vec4 computeBG(float mask)
{
    vec4 result;
    vec4 fgColor = texture2D(texForeground, v_texCoord);
    vec4 bgColor = texture2D(texBackground, v_texCoord); // stage 1: bgCopy

    if (mask > 0) {
        result = bgColor;
    } else {
        result = fgColor;
    }

    return result;
}

// Stage 2
vec4 blurEffectH(float mask)
{
    if (mask == 0) {
        return texture2D(texForeground, v_texCoord);
    }

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

// Stage 3
vec4 blurEffectV(float mask)
{
    if (mask == 0) {
        return texture2D(texForeground, v_texCoord);
    }

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

vec4 pixelation(float mask)
{
    if (mask == 0) {
        return texture2D(texForeground, v_texCoord);
    }

    vec2 dpos = 5 * (1/textureSize);
    vec2 coord = vec2(dpos.x * floor(v_texCoord.x / dpos.x), dpos.y * floor(v_texCoord.y / dpos.y));
    return texture2D(texForeground, coord);
}

vec4 embossEffect(float mask)
{
    /*if (mask == 0) {
        gl_FragColor = texture2D(texForeground, v_texCoord);
        return;
    }*/

    vec2 onePixel = 1 / textureSize;
    vec4 color;
    color.rgb = vec3(0.5);
    color -= texture2D(texForeground, v_texCoord - onePixel) * 5.0;
    color += texture2D(texForeground, v_texCoord + onePixel) * 5.0;
    color.rgb = vec3((color.r + color.g + color.b) / 3.0);
    return vec4(color.rgb, 1);
}
