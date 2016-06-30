#version 330

// The final step of LPV:
// Calculate propagation contribution of neighboring cells

in vec3 TexCoord0;

layout (location = 0) out vec4 frag_rcoeff;
layout (location = 1) out vec4 frag_gcoeff;
layout (location = 2) out vec4 frag_bcoeff;

layout (location = 3) out vec4 frag_raccum;
layout (location = 4) out vec4 frag_gaccum;
layout (location = 5) out vec4 frag_baccum;

uniform sampler3D RCoeff;
uniform sampler3D GCoeff;
uniform sampler3D BCoeff;

void main()
{
	const float falloff = 0.5;
	const float dimension = 32.0;

	const float pi = 3.14159;
	const float harmonic[] = float[](
		0.5 * sqrt(1.0 / pi),
		0.5 * sqrt(3.0 / pi)
	);
	const float lobe[] = float[](
		sqrt(pi) / 2.0,
		sqrt(pi / 3.0)
	);

	vec3 offsets[] = vec3[](
		vec3( 1.0,  0.0,  0.0),
		vec3(-1.0,  0.0,  0.0),
		vec3( 0.0,  1.0,  0.0),
		vec3( 0.0, -1.0,  0.0),
		vec3( 0.0,  0.0, -1.0),
		vec3( 0.0,  0.0,  1.0)
	);

	vec4 raccum = vec4(0.0);
	vec4 gaccum = vec4(0.0);
	vec4 baccum = vec4(0.0);

	for (int i = 0; i != 6; ++i)
	{
		vec3 coord = TexCoord0 + (offsets[i] / dimension);

		vec4 rcoeff = texture(RCoeff, coord);
		vec4 gcoeff = texture(GCoeff, coord);
		vec4 bcoeff = texture(BCoeff, coord);

		vec4 solid = vec4(
			 harmonic[0],
			-harmonic[1] * offsets[i].y,
			 harmonic[1] * offsets[i].z,
			-harmonic[1] * offsets[i].x
		);

		vec4 coeff = vec4(
			 lobe[0],
			-lobe[1] * offsets[i].y,
			 lobe[1] * offsets[i].z,
			-lobe[1] * offsets[i].x
		);

		raccum += max(dot(rcoeff, solid), 0.0) * coeff;
		gaccum += max(dot(gcoeff, solid), 0.0) * coeff;
		baccum += max(dot(bcoeff, solid), 0.0) * coeff;
	}

	frag_rcoeff = raccum * falloff;
	frag_gcoeff = gaccum * falloff;
	frag_bcoeff = baccum * falloff;

	frag_raccum = raccum;
	frag_gaccum = gaccum;
	frag_baccum = baccum;
}
