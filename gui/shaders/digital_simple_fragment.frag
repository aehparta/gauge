#version 330 core

uniform vec4 cursor;
uniform vec4 color;
uniform bool test;

in float opacity;

void main()
{
    gl_FragDepth = 0.0;
    gl_FragColor = color;
    gl_FragColor.a *= opacity;
    if (test)
    {
        float cursor_distance = distance(gl_FragCoord.xy, cursor.zw);
        if (cursor_distance < 20.0)
        {
            gl_FragDepth = 1.0;
        }
    }
}
