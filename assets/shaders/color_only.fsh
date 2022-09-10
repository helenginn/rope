#version 300 es

precision lowp float;

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;

out vec4 FragColor;

void main()
{
	if (vColor[3] <= 0.01)
	{
		discard;
	}
	vec4 result = vColor;
	FragColor = result;
}



