#version 330 core

in vec2 TexCoord0;

out vec4 frag_color;

uniform vec3 Color;
uniform sampler2D Texture;

void main()
{
	frag_color = texture(Texture, TexCoord0) * vec4(Color, 1);
}
