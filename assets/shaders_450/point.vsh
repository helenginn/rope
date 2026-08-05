#version 330 core

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
out vec4 vExtra;
out vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	gl_Position = projection * model * pos;
	// normal.x is otherwise unused by point rendering - repurposed as an
	// optional per-vertex size scale about the shared uniform size,
	// defaulting to 0 so anything that never sets it renders at the same
	// uniform size as before.
	gl_PointSize = size * (1.0 + normal.x);
	mat3 rot = mat3(model);
	vExtra = extra;
	vTex = tex;
	vColor = color;
	vPos = vec4(mat3(model) * vec3(extra), 1.);
}


