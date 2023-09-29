#version 330 core

in vec4 vColor;
in vec3 vNormal;
in vec3 dPos;
in float jump;

uniform sampler2D pic_tex;
uniform float slice;
uniform int track_frag;

uniform vec3 centre;

out vec4 FragColor;

void main()
{
	float dist = length(dPos);
	if (track_frag == 1)
	{
		if (slice > 0.) discard;

		if (jump < 0.)
		{
			discard;
		}

		if (dist > 20. || dPos.z > 10.)
		{
			discard;
		}
	}

	float alpha = 1. - min(dist / 20., 1.);
	alpha = 1.;

	vec4 result = vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	remaining *= 0.8;
	vec3 unit = normalize(vNormal);

	if (track_frag == 0 && unit.z < 0.)
	{
		unit.z *= -1.f;
	}
	else if (unit.z < 0.)
	{
		discard;
	}
	remaining *= max(0., dot(unit, vec3(0., 0., 1.)));
	result.xyz += remaining;
	result.a = alpha;
	FragColor = result;
}



