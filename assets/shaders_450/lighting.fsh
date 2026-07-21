#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;

uniform sampler2D pic_tex;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;
layout (location = 2) out vec4 BrightColor;

void main()
{
	ValIndex = 0u;
	BrightColor = vec4(0.0);

	vec4 result = vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	remaining *= 0.5;
	vec3 unit = normalize(vNormal);
	remaining *= abs(dot(unit, vec3(0, 0, 1)));
	result.xyz += remaining;
	if (result.a <= 0.01)
	{
		discard;
	}
	FragColor = result;
}



