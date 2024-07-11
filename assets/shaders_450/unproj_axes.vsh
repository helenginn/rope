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
	vec3 n = normal;
	n.z = 0.;

	mat2 turn = mat2(n.x, n.y, n.y, -n.x);
	turn[1] *= 0.25;
//	pos.xy += turn * vec2(0., tex.x * 0.01);
	pos.xy += vec2(0., tex.x * 0.01);
	vTex.xy = tex.xy;
	vTex.x += 0.5;
	vPos = pos;
	gl_Position = pos;
	vColor = color;
	vExtra = extra;
}



