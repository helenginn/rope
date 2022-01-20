#version 330 core

in vec4 vPos;
in vec2 vTex;

uniform sampler2D pic_tex;

out vec4 FragColor;

void main()
{
	vec4 result = texture(pic_tex, vTex);
	FragColor = result;
}


