#version 300 es

precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;

uniform float fraction;
uniform sampler2D pic_tex;

out vec4 FragColor;

void main()
{
	vec4 result = texture(pic_tex, vTex);
	result += vColor;
	if (vTex.x < fraction)
	{
		result.rgb *= 0.5;
		result.b = 1.;
	}
	FragColor = result;
}


