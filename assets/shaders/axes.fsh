#version 300 es
precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;
in vec4 vExtra;

uniform sampler2D pic_tex;

out vec4 FragColor;

void main()
{
	vec4 result = texture(pic_tex, vTex);
	FragColor = result;
}




