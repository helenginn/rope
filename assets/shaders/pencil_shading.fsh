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
	vec4 result = vColor;
	vec3 remaining = vec3(1., 1., 1.) - result.xyz;
	vec3 unit = normalize(vNormal);
	float light_mod = abs(dot(unit, vec3(0, 0, 1)));
	remaining *= 0.5 * light_mod;

	result.xyz += remaining;
	result.xyz *= light_mod;
	if (result.a <= 0.01)
	{
		discard;
	}
	vec2 screen = gl_FragCoord.xy;
	vec2 size = vec2(textureSize(pic_tex, 0));
	screen /= size;
	screen = vec2(fract(screen.x), fract(screen.y));

	vec4 shading = texture(pic_tex, screen);
	result *= 0.8;
	result += shading;
}




