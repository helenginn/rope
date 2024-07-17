#version 300 es
precision lowp float;

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

uniform mat4 projection;
uniform mat4 model;
uniform float thickness;

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
	normalize(n);

	mat2 turn = mat2(n.x, n.y, n.y, -n.x);
	pos.xy += turn * vec2(0., tex.x * thickness);
	vTex.xy = tex.xy;
	vTex.x += 0.5;
	vPos = pos;
	gl_Position = pos;
	vColor = color;
	vExtra = extra;
}




