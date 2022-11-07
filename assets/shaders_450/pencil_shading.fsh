#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;

uniform float dot_threshold;
uniform sampler2D pic_tex;

out vec4 FragColor;

void main()
{
	vec4 result = vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	vec3 unit = normalize(vNormal);
	float dot_mod = dot(unit, vec3(0, 0, 1));
	float light_mod = abs(dot_mod);
	remaining *= 0.5 * light_mod;

	if (dot_threshold < -1.0 && dot_mod < 0)
	{
		discard;
	}

	result.xyz += remaining;
	result.xyz *= light_mod;

	vec2 screen = gl_FragCoord.xy;
	vec2 size = vec2(textureSize(pic_tex, 0));
	screen /= size;
	screen = vec2(fract(screen.x), fract(screen.y));

	vec4 shading = texture(pic_tex, screen);
	float val = (shading.x + shading.y + shading.z) / 3.;

	result.xyz *= 0.8;
	result.xyz += shading.xyz;
	if (dot_mod < dot_threshold && dot_mod > -0.5 && val > 0.10)
	{
		result.xyz *= 1.5 + val;
	}

	result.a = 1.;
	FragColor = result;
}



