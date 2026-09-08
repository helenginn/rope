#version 300 es
precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;
in vec3 vNormal;

uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;

out vec4 FragColor;

void main()
{
	vec2 tex = vec2(vNormal.x + vTex.x * vNormal.y, vTex.y * vNormal.z);
	vec4 result = texture(pic_tex, tex);

	if (result.a < 0.05)
	{
		discard;
	}
	if (vColor.a < -0.99)
	{
		discard;
	}

	if ((near_slab >= -1. && far_slab >= -1.) &&
		(vPos.z > far_slab || vPos.z < near_slab))
	{
		discard;
	}

	result += vColor;
	FragColor = result;
}
