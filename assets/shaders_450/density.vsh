#version 330 core

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

uniform mat4 projection;
uniform mat4 model;
uniform vec3 centre;
uniform mat3x3 unit_cell;

out vec4 vPos;
out vec3 dPos;
out vec4 vColor;
out vec3 vNormal;
out vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	vec3 diff = vec3(model * pos) - centre;
	diff = inverse(mat3(model)) * diff;

	vec3 basis_diff = inverse(unit_cell) * diff;

	for (int i = 0; i < 3; i++)
	{
		int sign = (basis_diff[i] > 0 ? 1 : -1);
		float abs_diff = abs(basis_diff[i]);
		int round = 0;

		while (abs_diff > 0.5)
		{
			abs_diff--;
			round++;
		}

		round *= sign;

		vec3 axis = round * unit_cell[i];
		pos -= vec4(axis, 0.f);
	}

	gl_Position = projection * model * pos;
	mat3 rot = mat3(model);
	vNormal = rot * normal;
	vTex = tex;
	vColor = color;
	vColor = vec4(basis_diff, 1.f);
	vPos = vec4(rot * vec3(extra), 1.);
	dPos = vec3(model * pos) - centre;
}


