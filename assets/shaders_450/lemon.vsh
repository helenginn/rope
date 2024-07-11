#version 330 core

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

uniform mat4 projection;
uniform mat4 model;
uniform vec3 centre;

out vec4 vPos;
out vec4 vColor;
out vec4 vExtra;
out vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	vPos = projection * model * pos;
	vec4 cPos = vec4(centre, 1);
	cPos.x = 0;
	cPos.y = 0;
	cPos.w = 0;

	float l = length(cPos);
	l /= 80.;
	l *= l;
	l = min(l, 1.);

	vPos.xyz += extra.xyz * l;
	mat3 rot = mat3(model);
	vTex = tex;
	vColor = color;
	vExtra = extra;
	gl_Position = vPos;
}

