#version 130
// 0: no filter; 1: invisibility; 2: blur; 3: pixelation
// 4: emboss, 5: silhouette, 6: skeleton
uniform int currentFilter;
varying highp vec4 theColor;

void main()
{
    if (currentFilter != 6)
        discard;

    gl_FragColor = theColor;
}
