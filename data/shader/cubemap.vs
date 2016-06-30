#version 330 core

layout (location = 0) in vec3 position;

out vec3 TexCoord0;

uniform mat4 VP;

void main()
{
	TexCoord0 = position;

	gl_Position = (VP * vec4(position, 1)).xyww;
}
