precision lowp float;
varying vec4 vPos;
varying vec4 vColor;
varying vec2 vTex;

uniform sampler2D pic_tex;

void main()
{
	vec4 result = texture2D(pic_tex, vTex);
	result *= vColor;
	gl_FragColor = result;
}


