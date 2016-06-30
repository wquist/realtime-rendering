#version 330

layout (location = 0) in vec4 position;

out float Visibility;

uniform mat4 M;
uniform mat4 VP;

uniform float Time;
uniform vec3 Min;
uniform vec3 Max;

void main()
{
	vec3 range = Max - Min;

	// Adjust the position based on time interpolated through sin+cos
	float y = mod(position.y - Time, range.y) - (range.y * 0.5) - Min.y;
	float x = position.x + 5.0 * sin(cos(y * 0.1) + sin(y * 0.1 + position.x * 0.1) * 2.0);
	float z = position.z + 5.0 * sin(cos(y * 0.1) + sin(y * 0.1 + position.z * 0.1) * 2.0);
	// Also adjust transparency in a similar manner
	float o = abs(sin((position.w - Time) * 0.1));

	Visibility   = o;
	gl_PointSize = 5.0;
	gl_Position  = VP * M * vec4(x, y, z, 1.0);
}
