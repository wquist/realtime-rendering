#version 330 core

in vec2 TexCoord0;

out vec3 frag_color;

uniform sampler2D Albedo;
uniform sampler2D Normal;
uniform sampler2D Depth;
uniform sampler2D Shadow;

uniform sampler3D LPV;
uniform sampler3D GIR;
uniform sampler3D GIG;
uniform sampler3D GIB;

uniform sampler2D Dust;
uniform sampler2D Air;

uniform mat4 VP;
uniform mat4 InvVP;
uniform mat4 Clip;
uniform vec3 CameraPos;
uniform mat4 LightVP;
uniform vec3 LightDir;

uniform int  Mode;
uniform bool Shadows;
uniform bool Occlusion;
uniform bool Reflections;
uniform bool Indirect;
uniform bool Godrays;
uniform int  Layer;

const int poisson_samples = 16;
const vec2 poisson[] = vec2[](
	vec2( -0.94201624, -0.39906216 ),
	vec2(  0.94558609, -0.76890725 ),
	vec2( -0.09418410, -0.92938870 ),
	vec2(  0.34495938,  0.29387760 ),
	vec2( -0.91588581,  0.45771432 ),
	vec2( -0.81544232, -0.87912464 ),
	vec2( -0.38277543,  0.27676845 ),
	vec2(  0.97484398,  0.75648379 ),
	vec2(  0.44323325, -0.97511554 ),
	vec2(  0.53742981, -0.47373420 ),
	vec2( -0.26496911, -0.41893023 ),
	vec2(  0.79197514,  0.19090188 ),
	vec2( -0.24188840,  0.99706507 ),
	vec2( -0.81409955,  0.91437590 ),
	vec2(  0.19984126,  0.78641367 ),
	vec2(  0.14383161, -0.14100790 )
);

const int dither_rings = 4;
const float dither[] = float[](
	0.0000f, 0.5000f, 0.1250f, 0.6250f,
	0.7500f, 0.2200f, 0.8750f, 0.3750f,
	0.1875f, 0.6875f, 0.0625f, 0.5625f,
	0.9375f, 0.4375f, 0.8125f, 0.3125f
);

vec3 transform(vec3 p, mat4 space)
{
	vec4 proj = space * vec4(p, 1.0);
	return (proj.xyz / proj.w);
}

vec3 calculate_position(vec2 coord, sampler2D depth, mat4 inv)
{
	vec2 xy = coord * 2.0 - 1.0;
	float z = texture(depth, coord).r * 2.0 - 1.0;

	return transform(vec3(xy, z), inv);
}

bool depth_test(float z, vec2 coord, sampler2D depth)
{
	float target = texture(depth, coord).r;
	return (target > z);
}

vec3 ambient()
{
	const float term = 0.05;

	return vec3(term);
}

vec3 diffuse(vec3 normal)
{
	return vec3(max(dot(normal, -LightDir), 0.0));
}

// (specular)
vec3 highlight(vec3 position, vec3 normal, float specular)
{
	const int intensity = 64;

	vec3 view = normalize(CameraPos - position);
	vec3 refl = normalize(reflect(LightDir, normal));

	float spec = max(dot(view, refl), 0.0);
	return vec3(specular) * pow(spec, intensity);
}

// Idea and poisson samples from:
// http://blog.evoserv.at/index.php/2012/12/hemispherical-screen-space-ambient-occlusion-ssao-for-deferred-renderers-using-openglglsl/
// Use poisson samples instead of the normal method involving blur
float ssao(vec3 position, vec3 normal)
{
	const float radius = 0.01;
	const float threshold = 0.3;

	float total = 0.0;
	for (int i = 0; i != poisson_samples; ++i)
	{
		vec2 coord  = TexCoord0 + (poisson[i] * radius);
		vec3 sample = calculate_position(coord, Depth, InvVP);

		vec3  dir   = normalize(sample - position);
		float angle = max(dot(normal, dir), 0.0);
		float dist  = distance(position, sample);

		float factor = 1.0 - smoothstep(threshold, threshold * 2.0, dist);
		total += (factor * angle);
	}

	return 1.0 - (total / float(poisson_samples));
}

// Similar to idea from:
// http://casual-effects.blogspot.com/2014/08/screen-space-ray-tracing.html
// Cast low-resolution rays to accumulate the reflection color
// Adjust based on distance, reflection angle, and screen edge closeness
vec3 ssr(vec3 position, vec3 normal, float specular)
{
	const int samples = 8;
	const float radius = 0.1;
	const float smoothness = 0.5;
	const float fade = 15.0;
	const float falloff = 0.7;
	const float ramp = 4.0;

	vec3 view = normalize(CameraPos - position);
	vec3 ray  = normalize(reflect(-view, normal));

	vec3 total = vec3(0.0);
	for (int i = 0; i != samples; ++i)
	{
		vec3 sample = position + (ray * float(i) * radius);
		vec3 clip   = transform(sample, VP) * 0.5 + 0.5;

		vec3 sdist = sample - calculate_position(clip.xy, Depth, InvVP);
		vec3 rdist = sample - position;

		float bx = max(smoothness - (length(sdist) / fade), 0.0);
		float by = max(length(rdist) / falloff, 1.0);

		vec2 factor = pow(abs(clip.xy * 2.0 - 1.0), vec2(ramp));
		float edge  = 1.0 - clamp(factor.x + factor.y, 0.0, 1.0);

		vec3 color = texture(Albedo, clip.xy).rgb;
		total += color * edge * bx / by;
	}

	float angle = pow(1.0 - max(dot(view, normal), 0.0), 2);
	return total * angle * specular;
}

// Idea from the Crytek paper
// Get lighting contributions from spherical harmonic propagation results
vec3 gi(vec3 position, vec3 normal)
{
	const float power = 0.3;
	const float cutoff = 0.2;
	const float dimension = 32.0;

	const float pi = 3.14159;
	const float harmonic[] = float[](
		0.5 * sqrt(1.0 / pi),
		0.5 * sqrt(3.0 / pi)
	);

	vec4 solid = vec4(
		 harmonic[0],
		-harmonic[1] * normal.y,
		 harmonic[1] * normal.z,
		-harmonic[1] * normal.x
	);

	vec3 lpv_pos = transform(position, Clip);
	float gir = max(dot(solid, texture(GIR, lpv_pos)), 0.0);
	float gig = max(dot(solid, texture(GIG, lpv_pos)), 0.0);
	float gib = max(dot(solid, texture(GIB, lpv_pos)), 0.0);

	return min(vec3(gir, gig, gib) * power, vec3(cutoff));
}

// PCF filtering on shadows
float shadow(vec3 position)
{
	const int rings = 2;
	const float scale = 0.0003;
	const float bias = 0.0005;

	vec3 coord = transform(position, LightVP) * 0.5 + 0.5;

	float total = 0.0;
	for (int x = -rings; x <= rings; ++x)
	{
		for (int y = -rings; y <= rings; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * scale;
			vec2 sample = coord.xy + offset;

			if (!depth_test(coord.z - bias, sample, Shadow))
				total += 1.0;
		}
	}

	float total_samples = pow(2 * rings + 1, 2);
	return (total / float(total_samples));
}

// Calculate a fog coefficient based on distance and light angle
// Add dithering using a bayer matrix
// Modulate by raycasting through shadow map for visibilty
// Based off of ideas in:
// http://www.alexandre-pestana.com/volumetric-lights/
vec3 scattering(vec3 position)
{
	const int samples = 16;
	const float scatter = 0.3;
	const float thickness = 2.0;
	const float intensity = 2.5;
	const float power = 1.5;
	const float falloff = 0.1;
	const float threshold = 0.2;
	const float visibility = 0.7;

	const float pi = 3.14159;

	vec3 view  = position - CameraPos;
	vec3 ray   = normalize(view);
	vec3 rstep = ray * (length(view) / float(samples));

	float fog   = pow(1.0 - scatter, thickness);
	float angle = min(dot(ray, -LightDir) + visibility, 1.0);
	float phase = 1.0 + (scatter * scatter) - (2.0 * scatter * angle);
	float media = fog / (4.0 * pi * pow(phase, intensity));

	int dx = int(gl_FragCoord.x) % dither_rings;
	int dy = int(gl_FragCoord.y) % dither_rings;
	int di = (dy * dither_rings) + dx;

	vec3 sample = position - (rstep * dither[di]);
	vec3 close  = position;
	float accum = 0.0;

	for (int i = 0; i != samples; ++i)
	{
		vec3 coord = transform(sample, LightVP) * 0.5 + 0.5;
		if (depth_test(coord.z, coord.xy, Shadow))
		{
			accum += media;
			close  = sample;
		}

		sample -= rstep;
	}

	float fade = min((distance(CameraPos, close) * falloff) + threshold, 1.0);
	return vec3(accum / float(samples)) * power * fade;
}

// Fade the dust particles depending on camera distance
vec3 dust()
{
	const float falloff = 3.0;
	const float power = 3.0;

	vec3 position = calculate_position(TexCoord0, Air, InvVP);
	vec3 coord    = transform(position, LightVP) * 0.5 + 0.5;

	float amount = depth_test(coord.z, coord.xy, Shadow) ? 1.0 : 0.0;
	float atten  = max(distance(CameraPos, position) - falloff, 1.0);

	vec3 color = texture(Dust, TexCoord0).rgb;
	return color * power * (amount / atten);
}

void main()
{
	const float dimension = 32.0;

	vec3 position  = calculate_position(TexCoord0, Depth, InvVP);
	vec3 normal    = texture(Normal, TexCoord0).rgb * 2.0 - 1.0;
	vec3 color     = texture(Albedo, TexCoord0).rgb;
	float specular = texture(Albedo, TexCoord0).a;
	float depth    = texture(Depth, TexCoord0).r;

	float ao    = (Occlusion) ? ssao(position, normal) : 1.0;
	float illum = (Shadows) ? 1.0 - shadow(position) : 1.0;

	vec3 ambient   = ambient() * ao;
	vec3 diffuse   = diffuse(normal) * illum;
	vec3 highlight = highlight(position, normal, specular) * illum;
	vec3 gi        = (Indirect) ? gi(position, normal) * ao : vec3(0.0);

	// "sheen" is the variable to hold reflection color
	float drefl = (Reflections) ? min(illum, 0.4) : 0.0;
	float irefl = (Reflections) ? min(min(length(gi), 1.0), 0.1) : 0.0;
	vec3 sheen  = ssr(position, normal, specular) * (drefl + irefl);

	vec3 rays  = (Godrays) ? scattering(position) : vec3(0.0);
	vec3 dust  = dust() * rays;
	vec3 media = rays + dust;

	frag_color   = color * (ambient + diffuse + gi) + highlight + sheen + media;
	gl_FragDepth = depth;

	float vol_layer = (float(Layer) + 0.5) / dimension;
	switch (Mode)
	{
		case 1:
			frag_color = color;
			break;
		case 2:
			frag_color = vec3(specular);
			break;
		case 3:
			frag_color = position * 0.5 + 0.5;
			break;
		case 4:
			frag_color = normal * 0.5 + 0.5;
			break;
		case 5:
			frag_color = vec3(ao);
			break;
		case 6:
			frag_color = sheen;
			break;
		case 7:
			frag_color = texture(Shadow, TexCoord0).rrr;
			break;
		case 8:
			frag_color = texture(LPV, vec3(TexCoord0, vol_layer)).rgb;
			break;
		case 9:
			frag_color = texture(GIR, vec3(TexCoord0, vol_layer)).rgb;
			break;
	}
}
