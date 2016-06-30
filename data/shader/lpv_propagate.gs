#version 330

layout (points) in;
layout (points, max_vertices = 1) out;

in vec3 _TexCoord0[1];

out vec3 TexCoord0;

void main()
{
	const int dimension = 32;

	vec4 pos  = gl_in[0].gl_Position;
	TexCoord0 = _TexCoord0[0];

	gl_Position = vec4(pos.xy * 2.0 - 1.0, 0.0, 1.0);
	gl_Layer    = int(pos.z * dimension);

	EmitVertex();
	EndPrimitive();
}
