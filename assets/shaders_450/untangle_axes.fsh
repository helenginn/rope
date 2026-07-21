#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;
in vec4 vExtra;

uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;
uniform bool show_dirt;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;
layout (location = 2) out vec4 BrightColor;

void main()
{
	BrightColor = vec4(0.0);

	vec4 result = texture(pic_tex, vTex);

	if (result.a < 0.05)
	{
		discard;
	}
	if (vColor.a < -0.99)
	{
		discard;
	}

	if ((near_slab >= -1 && far_slab >= -1) &&
		(vPos.z > far_slab || vPos.z < near_slab))
	{
		discard;
	}

	result += vColor;

	if (show_dirt)
	{
		result.xyz += vExtra[0];
	}
	result[3] = vColor[3];
	ValIndex = uint(vExtra.w);
	FragColor = result;
}



