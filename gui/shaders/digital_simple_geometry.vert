#version 400 core

layout(lines) in;
layout(line_strip, max_vertices = 3) out;

void main()
{
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    if (gl_in[0].gl_Position.y != gl_in[1].gl_Position.y)
    {
        gl_Position.x = gl_in[1].gl_Position.x;
        EmitVertex();
    }
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();
    EndPrimitive();
}
