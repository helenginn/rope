#version 300 es
precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;

uniform float slice;
uniform sampler2D pic_tex;

out vec4 FragColor;

void main()
{
	if (slice > 0. && vPos.z > 0.) discard;
	vec4 result = texture(pic_tex, vTex);
	result += vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	remaining *= 0.5;
	vec3 unit = normalize(vNormal);
	remaining *= max(0., dot(unit, vec3(-0.707, 0.000, -0.707)));
	result.xyz += remaining;
	FragColor = result;
}



