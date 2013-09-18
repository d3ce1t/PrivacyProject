#version 130
varying highp float v_distance;

float normalise(float value, float minValue, float maxValue, float newMin, float newMax);
vec4 hsv_to_rgb(float h, float s, float v, float a);

void main()
{
    float norm_color = normalise(v_distance, 0.9, 6, 0, 0.85);

    if (norm_color > 0.9)
        norm_color = 0.9;

    gl_FragColor = hsv_to_rgb(norm_color, 1.0, 1.0, 1.0);
}

vec4 hsv_to_rgb(float h, float s, float v, float a)
{
        float c = v * s;
        h = mod((h * 6.0), 6.0);
        float x = c * (1.0 - abs(mod(h, 2.0) - 1.0));
        vec4 color;

        if (0.0 <= h && h < 1.0) {
                color = vec4(c, x, 0.0, a);
        } else if (1.0 <= h && h < 2.0) {
                color = vec4(x, c, 0.0, a);
        } else if (2.0 <= h && h < 3.0) {
                color = vec4(0.0, c, x, a);
        } else if (3.0 <= h && h < 4.0) {
                color = vec4(0.0, x, c, a);
        } else if (4.0 <= h && h < 5.0) {
                color = vec4(x, 0.0, c, a);
        } else if (5.0 <= h && h < 6.0) {
                color = vec4(c, 0.0, x, a);
        } else {
                color = vec4(0.0, 0.0, 0.0, a);
        }

        color.rgb += v - c;

        return color;
}

float normalise(float value, float minValue, float maxValue, float newMin, float newMax)
{
    return ( (value - minValue) * (newMax - newMin) ) / (maxValue - minValue) + newMin;
}
