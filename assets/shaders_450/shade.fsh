#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;

uniform sampler2D pic_tex;

out vec4 FragColor;

void main()
{
	FragColor = vColor;
}


