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
out vec3 vNormal;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	vPos = projection * model * pos;
	vPos.xyz += extra.xyz;
	vTex = tex;
	vColor = color;
	vExtra = extra;
	// same as floating_box.vsh, plus passing the atlas sub-rect through -
	// normal is otherwise unused by this pipeline (ProbeAtomBatch stores
	// each slot own atlas sub-rect here: u0, u-span, v-span, since extra
	// is already fully spent on the billboard offset + GPU-pick id).
	vNormal = normal;
	gl_Position = vPos;
}
