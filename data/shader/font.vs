#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

out vec2 TexCoord0;

uniform mat4 MVP;

void main()
{
	TexCoord0 = texcoord;

	gl_Position = MVP * vec4(position, 0, 1);
} 
