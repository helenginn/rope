#version 300 es
precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;
in vec4 vExtra;

uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;

out vec4 FragColor;

void main()
{
	vec4 result = texture(pic_tex, vTex);

	if (result.a < 0.05)
	{
		discard;
	}

	if (vPos.z > far_slab || vPos.z < near_slab)
	{
		discard;
	}

	result += vColor;
	FragColor = result;
}




