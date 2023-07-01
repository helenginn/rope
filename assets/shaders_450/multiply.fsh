#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;
in vec4 vExtra;

uniform sampler2D pic_tex;

layout (location = 0) out vec4 FragColor;
layout (location = 2) out vec4 BrightColor;

vec4 get_glow(float prop)
{
	vec4 colour;
	colour.r = prop > 1 ? prop - 1 : 0;
	colour.g = colour.r > 1 ? colour.r - 1 : 0;
	colour.b = colour.g > 1 ? colour.g - 1 : 0;
	colour.r = min(colour.r, 1.);
	colour.g = min(colour.g, 1.);
	colour.b = min(colour.b, 1.);
	colour.a = 1;
	return colour;
}


void main()
{
	vec4 result = texture(pic_tex, vTex);
	vec4 color = vColor;
	color.a = result.a;
	FragColor = color;
	BrightColor = get_glow(vExtra[3]);
}


