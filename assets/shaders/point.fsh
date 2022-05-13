precision lowp float;
varying vec4 vPos;
varying vec4 vColor;
varying vec2 vTex;

uniform sampler2D pic_tex;

void main()
{
	vec2 frag = gl_PointCoord;
	vec2 xy = vec2(frag[0], frag[1]);
	xy.x /= vTex.x;				// stores number of points
	xy.x += vTex.y / vTex.x;	// stores point index
	vec4 tex = texture2D(pic_tex, xy);
	if (tex[3] < 0.05)
	{
		discard;
	}

	vec4 temp = vColor;
	temp.a *= tex.a;
	gl_FragColor = temp;
}


