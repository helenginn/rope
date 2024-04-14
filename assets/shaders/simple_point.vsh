#version 300 es

precision lowp float;

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

uniform float size;
uniform mat4 projection;
uniform mat4 model;

out vec4 vProper;
out vec4 vPos;
out vec4 vColor;
out vec4 vExtra;
out vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	gl_Position = projection * model * pos;
	gl_PointSize = 50. * size / gl_Position.w;
	mat3 rot = mat3(model);
	vExtra = extra;
	vTex = tex;
	vColor = color;
	vProper = gl_Position;
	vPos = vec4(mat3(model) * vec3(extra), 1.);
}


