#version 330

out vec3 _TexCoord0;

void main()
{
	const int dimension = 32;

	int x = gl_VertexID % dimension;
	int y = gl_VertexID / dimension % dimension;
	int z = gl_VertexID / dimension / dimension;
	vec3 coord = (vec3(x, y, z) + 0.5) / float(dimension);

	_TexCoord0  = coord;
	gl_Position = vec4(coord, 1.0);
}
