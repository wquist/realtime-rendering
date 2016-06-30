#version 330

out vec2 _TexCoord0;
out vec3 _Normal0;

uniform sampler2D Normal;
uniform sampler2D Depth;

uniform mat4 InvLightVP;
uniform mat4 Clip;

vec3 calculate_position(vec2 coord)
{
	vec2 xy = coord * 2.0 - 1.0;
	float z = texture(Depth, coord).r * 2.0 - 1.0;

	vec4 proj = vec4(xy, z, 1.0);
	vec4 pos  = InvLightVP * proj;

	return (pos.xyz / pos.w);
}

void main()
{
	const int count = 1024;
	const float dimension = 32.0;

	int x = gl_VertexID % count;
	int y = gl_VertexID / count;
	vec2 coord = vec2(x, y) / float(count);

	vec3 position = calculate_position(coord);
	vec3 normal   = texture(Normal, coord).xyz * 2.0 - 1.0;

	vec4 space = Clip * vec4(position, 1.0);

	vec3 lpv_pos  = space.xyz / space.w;
	lpv_pos += normal * (0.5 / dimension);

	_TexCoord0  = coord;
	_Normal0    = normal;
	gl_Position = vec4(lpv_pos, 1.0);
}
