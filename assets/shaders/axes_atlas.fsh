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
	// same as axes.fsh, except pic_tex holds several images side by side
	// (an atlas) rather than one - vExtra.x/y/z (unused by axes.fsh) give
	// this vertex's slot its own sub-rect within it: u0, u-span, v-span
	// (v0 is always 0. - IndexedBatch/Library::buildAtlas() only ever
	// packs a single horizontal row), so a single draw call can render
	// many differently-imaged instances (see ProbeBondBatch).
	vec2 tex = vec2(vExtra.x + vTex.x * vExtra.y, vTex.y * vExtra.z);
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

	result.xyz += vColor.xyz;
	result.a *= (1. + vColor.a);
	FragColor = result;
}
