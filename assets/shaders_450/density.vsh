#version 330 core

in vec3 position;
in vec3 second;
in vec3 third;
in vec3 normal;
in vec4 color;

uniform mat4 projection;
uniform mat4 model;
uniform vec3 centre;
uniform mat3x3 unit_cell;
uniform int track;

out vec3 dPos;
out vec4 vColor;
out vec3 vNormal;
out float jump;

mat3x3 inv_rot = inverse(mat3(model));
mat3x3 inv_cell = inverse(mat3(unit_cell));

ivec3 round_movement(vec3 v)
{
    vec4 pos = vec4(v, 1.);
	vec3 diff = vec3(model * pos) - centre;
	diff = inv_rot * diff;

	vec3 basis_diff = inv_cell * diff;
	ivec3 ret;

	for (int i = 0; i < 3; i++)
	{
		int sign = (basis_diff[i] > 0. ? 1 : -1);
		float abs_diff = abs(basis_diff[i]);
		int round = 0;

		while (abs_diff > 0.5)
		{
			abs_diff -= 1.;
			round++;
		}

		round *= sign;
		ret[i] = round;
	}

	return ret;
}

vec4 move_vertex(vec3 v)
{
	vec3 round = vec3(round_movement(v));
    vec4 pos = vec4(v, 1.);

	for (int i = 0; i < 3; i++)
	{
		vec3 axis = round[i] * unit_cell[i];
		pos -= vec4(axis, 0.);
	}

	pos.w = 1.;
	return pos;
}

bool bad_length(vec3 v)
{
	vec4 pos = move_vertex(v);
	float dist = length(vec3(model * pos) - centre);
	if (dist > 19.)
	{
		jump = -1.;
		return true;
	}

	return false;
}

void main()
{

	vec4 ref1 = move_vertex(position);

	if (track == 1)
	{
		jump = -1.;
		if (bad_length(position) || bad_length(second) || bad_length(third))
		{
			return;
		}

		vec4 ref2 = move_vertex(second);
		vec4 ref3 = move_vertex(third);

		if (length(ref3 - ref2) > 19. || length(ref2 - ref1) > 19. ||
				length(ref1 - ref3) > 19.)
		{
			return;
		}
	}
	else
	{
		ref1 = vec4(position, 1.);
	}

	jump = 1.;
	vec4 pos = ref1;
	dPos = vec3(model * pos) - centre;

	gl_Position = projection * model * pos;
	mat3 rot = mat3(model);
	vNormal = rot * normal;
	vColor = color;
}



