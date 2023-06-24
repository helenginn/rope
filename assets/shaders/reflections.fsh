#version 300 es
precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;

uniform float slice;
uniform sampler2D pic_tex;

out vec4 FragColor;

void main()
{
	if (slice > 0. && vPos.z > 0.) discard;

	vec2 frag = gl_PointCoord;
	vec2 xy = vec2(frag[0], frag[1]);
	vec4 tex = texture(pic_tex, xy);
	if (tex.a < 0.2 || vColor.a < 0.01)
	{
		discard;
	}
	tex *= vColor;
	FragColor = tex;
}



