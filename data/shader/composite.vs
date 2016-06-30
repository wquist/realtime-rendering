#version 330 core

layout (location = 0) in vec2 position;

out vec2 TexCoord0;

const vec2 tex_scale = vec2(0.5, 0.5);

void main()
{
	TexCoord0 = position * tex_scale + tex_scale;

	gl_Position = vec4(position, 0, 1);
}
