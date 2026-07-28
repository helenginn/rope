#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;

uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;
layout (location = 2) out vec4 BrightColor;

void main()
{
	if ((near_slab >= -1 && far_slab >= -1) &&
		(vPos.z > far_slab || vPos.z < near_slab))
	{
		discard;
	}

	vec4 result = texture(pic_tex, vTex);
	if (result.a < 0.01)
	{
		discard;
	}

	result += vColor;
	FragColor = result;
	ValIndex = 0u;
	BrightColor = vec4(0.0);

}


