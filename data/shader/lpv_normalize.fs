#version 330

// Get the number of RSM injects per cell, and normalize intensity

in vec3 TexCoord0;

layout (location = 0) out vec4 frag_rcoeff;
layout (location = 1) out vec4 frag_gcoeff;
layout (location = 2) out vec4 frag_bcoeff;

uniform sampler3D RCoeff;
uniform sampler3D GCoeff;
uniform sampler3D BCoeff;
uniform sampler3D InjectCount;

void main()
{
	const float dimension = 32;

	float count = texture(InjectCount, TexCoord0).r * dimension;
	frag_rcoeff = texture(RCoeff, TexCoord0) / count;
	frag_gcoeff = texture(GCoeff, TexCoord0) / count;
	frag_bcoeff = texture(BCoeff, TexCoord0) / count;
}
