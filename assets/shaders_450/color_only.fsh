#version 330 core

in vec4 vProper;
in vec4 vPos;
in vec4 vColor;
in vec2 vTex;
in vec4 vExtra;

uniform sampler2D pic_tex;
uniform float near_slab;
uniform float far_slab;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;
layout (location = 2) out vec4 BrightColor;

void main()
{
	if ((near_slab >= -1 && far_slab >= -1) &&
		(vProper.z > far_slab || vProper.z < near_slab))
	{
		discard;
	}

	if (vColor.a <= 0.01)
	{
		discard;
	}

	vec4 result = vColor;
	FragColor = result;

	/* also rendered into multi-target (bloom) scenes: write the index
	 * and bright buffers so they don't take undefined values */
	ValIndex = 0u;
	BrightColor = vec4(0.0);
}



