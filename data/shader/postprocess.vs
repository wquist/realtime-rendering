#version 330 core

layout (location = 0) in vec2 position;

out vec2 TexCoord0;

void main()
{
	TexCoord0 = position * 0.5 + 0.5;

	gl_Position = vec4(position, 0, 1);
}
