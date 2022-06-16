#version 330 core

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

uniform mat4 projection;
uniform mat4 model;

out vec4 vPos;
out vec4 vColor;
out vec3 vNormal;
out vec2 vTex;
out vec4 vExtra;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	vec4 mPos = model * pos;
	mat3 rot = mat3(model[0][0], model[0][1], model[0][2],
				    model[1][0], model[1][1], model[1][2],
				    model[2][0], model[2][1], model[2][2]);

	vec3 n = rot * normal;
	n.z = 0.;

	mat2 turn = mat2(n.x, n.y, n.y, -n.x);
	turn[1] *= 0.5;
	mPos.xy += turn * tex.yx;
	vTex.xy = tex.xy;
	vTex.x += 0.5;
	vPos = projection * mPos;
	gl_Position = vPos;
	vColor = color;
	vExtra = extra;
}



