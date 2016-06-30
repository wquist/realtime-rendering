#version 330 core

out vec3 frag_color;

uniform vec3 PickColor;

void main()
{
	frag_color = PickColor;
}
