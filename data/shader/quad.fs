#version 330 core

in vec2 TexCoord0;

out vec3 frag_color;

uniform sampler2D Color;

void main()
{
	frag_color = texture(Color, TexCoord0).rgb;
}
