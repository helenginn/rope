#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;
in vec4 vExtra;

uniform sampler2D pic_tex;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;

void main()
{
	vec4 result = texture(pic_tex, vTex);

	if (result.a < 0.05)
	{
		discard;
	}
	result += vColor;
	ValIndex = uint(vExtra.x);
	FragColor = result;
}



