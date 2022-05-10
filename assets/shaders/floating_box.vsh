attribute vec3 normal;
attribute vec3 position;
attribute vec4 color;
attribute vec4 extra;
attribute vec2 tex;

uniform mat4 projection;
uniform mat4 model;

varying vec4 vPos;
varying vec4 vColor;
varying vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	vPos = projection * model * pos;
	vPos += extra;
	vTex = tex;
	vColor = color;
	gl_Position = vPos;
}

