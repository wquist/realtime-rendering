#version 330 core

in vec3 TexCoord0;

out vec4 frag_color;

uniform samplerCube CubeMap;

void main()
{
	frag_color = texture(CubeMap, TexCoord0);
}
