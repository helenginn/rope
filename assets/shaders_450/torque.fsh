#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;
in vec4 vExtra;

uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;

void main()
{
	vec2 tex = vTex;
	vec3 cross = cross(vNormal, vExtra.xyz);

	if (cross.z < 0)
	{
		tex.x = 1. - tex.x;
	}

	vec4 result = texture(pic_tex, tex);

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
	ValIndex = uint(vExtra.w);
	FragColor = result;
}



