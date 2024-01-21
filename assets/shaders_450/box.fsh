#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;

uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;

out vec4 FragColor;

void main()
{
	if ((near_slab >= -1 && far_slab >= -1) &&
		(vPos.z > far_slab || vPos.z < near_slab))
	{
		discard;
	}

	vec4 result = texture(pic_tex, vTex);
	result += vColor;
	FragColor = result;

}


