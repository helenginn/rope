#version 300 es
precision lowp float;

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

out vec4 vPos;
out vec4 vColor;
out vec2 vTex;
out vec4 vExtra;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	gl_Position = pos;
	gl_PointSize = 20.;
	vTex = tex;
	vExtra = extra;
	vColor = color;
}

