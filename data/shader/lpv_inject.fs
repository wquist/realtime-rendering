#version 330

// The first stage of LPV:
// Put the RSM info into the 3D textures as spherical harmonic coefficients

in vec2 TexCoord0;
in vec3 Normal0;

layout (location = 0) out vec4 frag_rcoeff;
layout (location = 1) out vec4 frag_gcoeff;
layout (location = 2) out vec4 frag_bcoeff;
layout (location = 3) out float frag_count;

uniform sampler2D Color;

void main()
{
	const float pi = 3.14159;
	const float lobe[] = float[](
		sqrt(pi) / 2.0,
		sqrt(pi / 3.0)
	);

	vec4 coeff = vec4(
		 lobe[0],
		-lobe[1] * -Normal0.y,
		 lobe[1] * -Normal0.z,
		-lobe[1] * -Normal0.x
	);

	vec3 flux = texture(Color, TexCoord0).rgb;
	frag_rcoeff = coeff * flux.r;
	frag_gcoeff = coeff * flux.g;
	frag_bcoeff = coeff * flux.b;

	frag_count = 1.0;
}
