#version 300 es

precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;

uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;

out vec4 FragColor;

void main()
{
	if ((near_slab >= -1 && far_slab >= -1) &&
		(vProper.z > far_slab || vProper.z < near_slab))
	{
		discard;
	}

	vec4 result = vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	vec3 unit = normalize(vNormal);
	float dot_mod = dot(unit, vec3(0, 0, 1));
	float light_mod = abs(dot_mod);
	remaining *= 0.5 * light_mod;

	result.xyz += remaining;
	result.xyz *= light_mod;

	vec2 screen = gl_FragCoord.xy;
	vec2 size = vec2(textureSize(pic_tex, 0));
	screen /= size;
	screen = vec2(fract(screen.x), fract(screen.y));

	vec4 shading = texture(pic_tex, screen);
	float val = (shading.x + shading.y + shading.z) / 3.;

	result *= 0.8;
	result += shading;
	if (dot_mod < 0.3 && dot_mod > -0.5 && val > 0.10)
	{
		result *= 1.5 + val;
	}

	result.a = 1.;
	FragColor = result;
}




