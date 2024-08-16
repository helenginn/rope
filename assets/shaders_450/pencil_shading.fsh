#version 330 core

in vec4 vProper;
in vec4 vPos;
in vec4 vColor;
in vec4 vExtra;
in vec3 vNormal;
in vec2 vTex;

uniform float dot_threshold;
uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;
layout (location = 2) out vec4 BrightColor;

vec4 get_base_colour(vec4 c)
{
	vec4 colour;
	colour.r = c.r > 1. ? 1. : c.r;
	colour.g = c.g > 1. ? 1. : c.g;
	colour.b = c.b > 1. ? 1. : c.b;
	colour.a = 1;
	return colour;
}

vec4 get_glow(float prop)
{
	vec4 colour;
	colour.r = prop > 1 ? prop - 1 : 0;
	colour.g = colour.r > 1 ? colour.r - 1 : 0;
	colour.b = colour.g > 1 ? colour.g - 1 : 0;
	colour.r = min(colour.r, 1.);
	colour.g = min(colour.g, 1.);
	colour.b = min(colour.b, 1.);
	colour.a = 1;
	return colour;
}

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

	FragColor = get_base_colour(result);
	FragColor.a = vColor.a;
	BrightColor = get_glow(vExtra[3]);
}



