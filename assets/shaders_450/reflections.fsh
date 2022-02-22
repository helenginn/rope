#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;

uniform sampler2D pic_tex;

out vec4 FragColor;

void main()
{
	if (vPos.z > 0.) discard;
	vec4 result = texture(pic_tex, vTex);
	result += vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	remaining *= 0.5;
	vec3 unit = normalize(vNormal);
	remaining *= abs(dot(unit, vec3(0, 0, 1)));
	result.xyz += remaining;
	FragColor = result;
}



