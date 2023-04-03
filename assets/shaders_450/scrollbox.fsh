#version 330 core

in vec4 vPos;
in vec4 vColor;
in vec2 vTex;

uniform sampler2D pic_tex;
uniform vec4 box;

out vec4 FragColor;

void main()
{
	vec4 pos = vPos;

	if (pos.y > box[0] || pos.y < box[2] ||
		pos.x < box[1] || pos.x > box[3])
	{
		discard;
	}

	vec4 result = texture(pic_tex, vTex);
	result += vColor;
	FragColor = result;
}


