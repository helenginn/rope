#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec3 vNormal;
in vec2 vTex;

uniform sampler2D pic_tex;
uniform float slice;

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



