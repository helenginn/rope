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

void main()
{
	if ((near_slab >= -1 && far_slab >= -1) &&
		(vProper.z > far_slab || vProper.z < near_slab))
	{
		discard;
	}

	vec2 frag = gl_PointCoord;
	vec2 xy = vec2(frag[0], frag[1]);
	vec4 tex = texture(pic_tex, xy);
	if (tex.a < 0.2 || vColor.a < 0.01)
	{
		discard;
	}
	tex *= vColor;
	ValIndex = uint(vExtra.x);
	FragColor = tex;
}


