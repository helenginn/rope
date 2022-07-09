#version 300 es

precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;

out vec4 FragColor;

void main()
{
	vec4 result = vColor;
	FragColor = result;
}



