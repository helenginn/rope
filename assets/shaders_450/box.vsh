#version 330 core

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

out vec4 vProper;
out vec4 vPos;
out vec4 vColor;
out vec4 vExtra;
out vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	vProper = pos;
	gl_Position = pos;
	gl_PointSize = 80;
	vTex = tex;
	vPos = pos;
	vColor = color;
	vExtra = extra;
}

