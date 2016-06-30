#version 330

in float Visibility;

out vec3 frag_color;

void main()
{
	frag_color = vec3(1.0) * Visibility;
}