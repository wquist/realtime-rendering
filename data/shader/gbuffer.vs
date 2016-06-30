#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal;

out vec2 TexCoord0;
out vec3 Normal0;

out vec3 Tangent;
out vec3 Bitangent;

uniform mat4 M;
uniform mat4 VP;

void main()
{
	TexCoord0 = texcoord;
	Normal0   = normalize((M * vec4(normal, 0.0)).xyz);

	Tangent   = normalize(cross(Normal0, normalize(vec3(-1.0))));
	Bitangent = normalize(cross(Normal0, Tangent));
	
	gl_Position = VP * M * vec4(position, 1.0);
}
