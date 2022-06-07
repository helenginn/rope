precision lowp float;

varying vec4 vPos;
varying vec4 vColor;
varying vec3 vNormal;
varying vec2 vTex;
in vec3 dPos;

uniform float slice;
uniform sampler2D pic_tex;
uniform vec3 centre;

void main()
{
	if (slice > 0. && vPos.z > 0.) discard;
	float dist = length(dPos);
	if (dist > 20.)
	{
		discard;
	}

	if (abs(dPos.z) > 10. || vNormal.z < 0.)
	{
		discard;
	}

	float alpha = 1 - min(dist / 20, 1);

	vec4 result = texture2D(pic_tex, vTex);
	result += vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	remaining *= 0.5;
	vec3 unit = normalize(vNormal);
	remaining *= max(0., dot(unit, vec3(0., 0.000, 1.0)));
	result.xyz += remaining;
	result.a = alpha;

	gl_FragColor = result;
}



