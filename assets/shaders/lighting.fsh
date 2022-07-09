#version 300 es

precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;

uniform sampler2D pic_tex;

out vec4 FragColor;

void main()
{
//	vec4 result = texture(pic_tex, vTex);
	vec4 result = vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	remaining *= 0.5;
	vec3 unit = normalize(vNormal);
	remaining *= max(dot(unit, vec3(0, 0, 1)), 0.);
	result.xyz += remaining;
	FragColor = result;
}



