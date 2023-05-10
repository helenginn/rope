#version 330 core

in vec4 vColor;
in vec3 vNormal;
in vec3 dPos;
in float jump;

uniform sampler2D pic_tex;
uniform float slice;

uniform vec3 centre;

out vec4 FragColor;

void main()
{
	if (slice > 0.) discard;

	if (jump < 0)
	{
		discard;
	}

	float dist = length(dPos);
	if (dist > 20. || dPos.z > 10)
	{
		discard;
	}

// ignore stuff facing away from the viewpoint
	if (vNormal.z < 0.)
	{
		discard;
	}

	float alpha = 1 - min(dist / 20, 1);
	alpha = 1;

	vec4 result = vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	remaining *= 0.8;
	vec3 unit = normalize(vNormal);
	remaining *= max(0., dot(unit, vec3(0., 0., 1.)));
	result.xyz += remaining;
	result.a = alpha;
	FragColor = result;
}



