attribute vec3 normal;
attribute vec3 position;
attribute vec4 color;
attribute vec4 extra;
attribute vec2 tex;

uniform mat4 projection;
uniform mat4 model;
uniform vec3 centre;

varying vec4 vPos;
varying vec3 dPos;
varying vec4 vColor;
varying vec3 vNormal;
varying vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	gl_Position = projection * model * pos;
	mat3 rot = mat3(model);
	vNormal = rot * normal;
	vTex = tex;
	vColor = color;
	vPos = vec4(mat3(model[0][0], model[0][1], model[0][2],
				model[1][0], model[1][1], model[1][2],
				model[2][0], model[2][1], model[2][2]) * vec3(extra), 1.);
	dPos = vec3(model * pos) - centre;
}


