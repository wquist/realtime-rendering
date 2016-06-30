#version 330 core

in vec2 TexCoord0;
in vec3 Normal0;

in vec3 Tangent;
in vec3 Bitangent;

layout (location = 0) out vec4 frag_albedo;
layout (location = 1) out vec4 frag_normal;

uniform sampler2D Color;
uniform sampler2D Alpha;
uniform sampler2D Normal;
uniform sampler2D Specular;

uniform bool NormalMap;

bool alpha_test()
{
	return (texture(Alpha, TexCoord0).r > 0.0);
}

// Cheap way to (inaccurately) calculate tangent+bitangent for perturbed normal
// Method similar to the one described at:
// http://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
vec3 calculate_normal()
{
	vec2 tdx = normalize(dFdx(TexCoord0));
	vec2 tdy = normalize(dFdy(TexCoord0));
	vec3 dx  = tdx.x * Tangent + tdx.y * Bitangent;
	vec3 dy  = tdy.x * Tangent + tdy.y * Bitangent;

	vec3 n = texture(Normal, TexCoord0).rgb * 2.0 - 1.0;
	return normalize(n.x * dx + n.y * dy + n.z * Normal0);
}

void main()
{
	if (!alpha_test())
		discard;

	vec3 normal = (NormalMap) ? calculate_normal() : Normal0;
	vec3 color  = texture(Color, TexCoord0).rgb;
	float spec  = texture(Specular, TexCoord0).r;

	frag_normal.xyz = normal * 0.5 + 0.5;
	frag_albedo.rgb = color;
	frag_albedo.a   = spec;
}
