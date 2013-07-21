#version 130
varying vec2 v_texCoord;
uniform int currentFilter; // 0: no filter; 1: invisibility; 2: blur
uniform int stage;
uniform sampler2D texForeground;
uniform sampler2D texMask;
uniform sampler2D texBackground;

vec4 computeBG(float mask);
vec4 embossEffect(float mask);
vec4 blurEffectH(float mask);
vec4 blurEffectV(float mask);

void main()
{
    float mask = texture2D(texMask, v_texCoord).r;
    vec4 fgColor = texture2D(texForeground, v_texCoord);
    vec4 bgColor = texture2D(texBackground, v_texCoord); // stage 1: bg, stage 2: fbo

    // input: fgColor, mask, bgColor
    if (stage == 1) {
        gl_FragColor = computeBG(mask);
    }
    // input: fgColor, mask, bgColor
    else if (stage == 2) {
        if (currentFilter == 0) {
            gl_FragColor = fgColor;
        }
        else if (currentFilter == 1) {
            gl_FragColor = bgColor;
        }
        else if (currentFilter == 2) {
            gl_FragColor = blurEffectH(mask);
        }
    }
    // input: mask, bgColor
    else {
        if (currentFilter == 2) {
            gl_FragColor = blurEffectV(mask);
        } else {
            gl_FragColor = fgColor;
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

    const float blurSize = 1.0/320.0;
    vec4 sum = vec4(0.0);

    // blur in y (vertical)
    // take nine samples, with the distance blurSize between them
    sum += texture2D(texForeground, vec2(v_texCoord.x - 4.0*blurSize, v_texCoord.y)) * 0.05;
    sum += texture2D(texForeground, vec2(v_texCoord.x - 3.0*blurSize, v_texCoord.y)) * 0.09;
    sum += texture2D(texForeground, vec2(v_texCoord.x - 2.0*blurSize, v_texCoord.y)) * 0.12;
    sum += texture2D(texForeground, vec2(v_texCoord.x - blurSize, v_texCoord.y)) * 0.15;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y)) * 0.16;
    sum += texture2D(texForeground, vec2(v_texCoord.x + blurSize, v_texCoord.y)) * 0.15;
    sum += texture2D(texForeground, vec2(v_texCoord.x + 2.0*blurSize, v_texCoord.y)) * 0.12;
    sum += texture2D(texForeground, vec2(v_texCoord.x + 3.0*blurSize, v_texCoord.y)) * 0.09;
    sum += texture2D(texForeground, vec2(v_texCoord.x + 4.0*blurSize, v_texCoord.y)) * 0.05;

    return sum;
}

// Stage 3
vec4 blurEffectV(float mask)
{
    if (mask == 0) {
        return texture2D(texForeground, v_texCoord);
    }

    const float blurSize = 1.0/240.0;
    vec4 sum = vec4(0.0);

    // blur in y (vertical)
    // take nine samples, with the distance blurSize between them
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y - 4.0*blurSize)) * 0.05;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y - 3.0*blurSize)) * 0.09;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y - 2.0*blurSize)) * 0.12;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y - blurSize)) * 0.15;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y)) * 0.16;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y + blurSize)) * 0.15;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y + 2.0*blurSize)) * 0.12;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y + 3.0*blurSize)) * 0.09;
    sum += texture2D(texForeground, vec2(v_texCoord.x, v_texCoord.y + 4.0*blurSize)) * 0.05;

    return sum;
}

vec4 embossEffect(float mask)
{
    /*if (mask == 0) {
        gl_FragColor = texture2D(texForeground, v_texCoord);
        return;
    }*/

    vec2 onePixel = vec2(1.0 / 640.0, 1.0 / 480.0);

    // 4
    vec4 color;
    color.rgb = vec3(0.5);
    color -= texture2D(texForeground, v_texCoord - onePixel) * 5.0;
    color += texture2D(texForeground, v_texCoord + onePixel) * 5.0;

    // 5
    color.rgb = vec3((color.r + color.g + color.b) / 3.0);
    return vec4(color.rgb, 1);
}
