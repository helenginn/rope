#version 300 es
precision lowp float;

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

uniform mat4 projection;
uniform mat4 model;

out vec4 vPos;
out vec4 vColor;
out vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	vPos = projection * model * pos;
	vPos += extra;
	vTex = tex;
	vColor = color;
	gl_Position = vPos;
	// pull labels a little towards the camera (a fraction of the clip depth,
	// so it holds at any distance) so they draw in front of what they label
	gl_Position.z -= 0.1 * gl_Position.w;
}

