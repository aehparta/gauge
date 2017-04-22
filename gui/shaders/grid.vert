#version 330 core

attribute vec2 point;

void main()
{
	gl_Position.xy = point;
	gl_Position.zw = vec2(0.0, 1.0);
}
