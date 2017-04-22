#version 330 core

uniform vec4 scale;

attribute float data;

out float opacity;

void main()
{
	gl_Position.x = scale.x + (float(gl_VertexID) * scale.z);
	gl_Position.y = scale.y + (data * scale.w);
	gl_Position.zw = vec2(0.0, 1.0);

	opacity = 1.0;

	if (gl_InstanceID > 0)
	{
		float diff = float((gl_InstanceID + 1) >> 1);
		opacity = 0.7 / diff;
		diff = (gl_InstanceID & 1) == 0 ? diff : -diff;
		gl_Position.x += diff / 1000.0;
		gl_Position.y += diff / 1000.0;
	}
}

