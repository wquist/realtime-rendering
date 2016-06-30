#version 330 core

in vec2 TexCoord0;

out vec3 frag_color;

uniform sampler2D Color;

void main()
{
	const float exposure = 1.29;

	vec3 color = texture(Color, TexCoord0).rgb;
	frag_color = pow(color, vec3(1.0 / exposure));
}
