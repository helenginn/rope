#version 330 core

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

out vec4 vPos;
out vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	gl_Position = pos;
    vPos = pos;
    vec2 tex = pos.xy + vec2(1., 1.);
	tex /= 2.;
	vTex = tex;
}

