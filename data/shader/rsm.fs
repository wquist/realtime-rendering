#version 330 core

// Sample scene color from light POV at each sample point
// Store in a texture along with normal and depth to create the reflection shadow map

in vec2 TexCoord0;
in vec3 Normal0;

layout (location = 0) out vec3 frag_albedo;
layout (location = 1) out vec3 frag_normal;

uniform sampler2D Color;

void main()
{
	frag_albedo = texture(Color, TexCoord0).rgb;
	frag_normal = Normal0 * 0.5 + 0.5;
}
