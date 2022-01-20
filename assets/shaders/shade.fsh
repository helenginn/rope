#version 320 es

precision lowp float;
varying vec4 vPos;
varying vec4 vColor;
varying vec2 vTex;

uniform sampler2D pic_tex;

void main()
{
	gl_FragColor = vColor;
}


