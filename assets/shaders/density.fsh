precision lowp float;

varying vec4 vPos;
varying vec4 vColor;
varying vec3 vNormal;
varying vec2 vTex;

uniform float slice;
uniform sampler2D pic_tex;

void main()
{
	if (slice > 0. && vPos.z > 0.) discard;
	vec4 result = texture2D(pic_tex, vTex);
	result += vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	remaining *= 0.5;
	vec3 unit = normalize(vNormal);
	remaining *= max(0., dot(unit, vec3(0., 0.000, -1.0)));
	result.xyz += remaining;
	gl_FragColor = result;
}


