attribute vec3 normal;
attribute vec3 position;
attribute vec4 color;
attribute vec4 extra;
attribute vec2 tex;

varying vec4 vPos;
varying vec2 vTex;

void main()
{
    vec4 pos = vec4(position[0], position[1], position[2], 1.0);
	gl_Position = pos;
    vPos = pos;
    vec2 tex = pos.xy + vec2(1., 1.);
	tex /= 2.;
	vTex = tex;
}

