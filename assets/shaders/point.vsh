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

out vec4 vPos;
out vec4 vColor;
out vec3 vNormal;
out vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	gl_Position = projection * model * pos;
	gl_PointSize = size / float(2);
	mat3 rot = mat3(model);
	vNormal = rot * normal;
	vTex = tex;
	vColor = color;
	vPos = vec4(mat3(model[0][0], model[0][1], model[0][2],
				model[1][0], model[1][1], model[1][2],
				model[2][0], model[2][1], model[2][2]) * vec3(extra), 1.);
}


