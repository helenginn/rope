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
out vec4 vExtra;
out vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	vPos = projection * model * pos;
	vPos.xyz += extra.xyz;
	mat3 rot = mat3(model);
	vTex = tex;
	vColor = color;
	vExtra = extra;
	gl_Position = vPos;
}

