#version 330 core

in vec4 vPos;
in vec2 vTex;

uniform sampler2D pic_tex;
uniform sampler2D bright_tex;

uniform int mode;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint ValIndex;

void main()
{
	float weight[5] = float[] (0.227027, 0.1945946, 0.1216216,
	                           0.054054, 0.016216);

	if (mode == 2)
	{
		vec3 result = texture(pic_tex, vTex).rgb;
		result += texture(bright_tex, vTex).rgb;

		const float exposure = 1.4;
		const float gamma = 1.5;
		// exposure tone mapping
		vec3 mapped = vec3(1.0) - exp(-result * result * exposure);

		// gamma correction 
		mapped = pow(mapped, vec3(1.0 / gamma));
		FragColor = vec4(mapped, 1.0);
	}
	else if (mode == 0 || mode == 1)
	{
		vec3 add = vec3(0., 0., 0.);
		ivec2 tex = ivec2(textureSize(bright_tex, 0) * vTex);

		for (int i = -4; i < 5; i++)
		{
			ivec2 off = ivec2(mode * i, (1 - mode) * i);
			vec3 pix = texelFetch(bright_tex, tex + off, 0).rgb;
			pix *= weight[abs(i)];

			add += pix;
		}

		FragColor = vec4(add, 1.0);
	}
	else if (mode == 3)
	{
		vec4 result = texture(pic_tex, vTex);
		FragColor = result;
	}

	FragColor.a = 1.;
}


