attribute vec3 normal;
attribute vec3 position;
attribute vec4 color;
attribute vec4 extra;
attribute vec2 tex;

varying vec4 vPos;
varying vec4 vColor;
varying vec2 vTex;
varying vec4 vExtra;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	gl_Position = pos;
	gl_PointSize = 20.;
	vTex = tex;
	vExtra = extra;
	vColor = color;
}

