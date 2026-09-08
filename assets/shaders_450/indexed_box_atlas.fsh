#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec4 vExtra;
in vec2 vTex;
in vec3 vNormal;

uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;
layout (location = 2) out vec4 BrightColor;

void main()
{
	if ((near_slab >= -1 && far_slab >= -1) &&
		(vPos.z > far_slab || vPos.z < near_slab))
	{
		discard;
	}

	if (vColor.a < -0.99)
	{
		discard;
	}

	// same as indexed_box.fsh, except pic_tex holds several glyphs side
	// by side (an atlas) rather than one - vNormal.x/y/z give this
	// vertex's slot its own sub-rect within it: u0, u-span, v-span (v0 is
	// always 0 - Library::buildTextAtlas() only ever packs a single
	// horizontal row), so a single draw call can render many
	// differently-labelled instances (see ProbeAtomBatch).
	vec2 tex = vec2(vNormal.x + vTex.x * vNormal.y, vTex.y * vNormal.z);
	vec4 result = texture(pic_tex, tex);

	if (result.a < 0.05)
	{
		discard;
	}

	result += vColor;
	ValIndex = uint(vExtra[3]);
	FragColor = result;
	BrightColor = vec4(0.0);
}
