#version 300 es

precision float lowp;

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;
in vec4 vExtra;

uniform sampler2D pic_tex;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;

void main()
{
	vec2 frag = gl_PointCoord;
	vec2 xy = vec2(frag[0], frag[1]);
	vec4 tex = texture(pic_tex, xy);
	temp *= tex;
	ValIndex = uint(vExtra.x);
	FragColor = tex;
}


