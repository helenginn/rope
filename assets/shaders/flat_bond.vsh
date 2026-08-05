#version 300 es
precision lowp float;

in vec3 normal;
in vec3 position;
in vec4 color;
in vec4 extra;
in vec2 tex;

uniform float aspect;

out vec4 vPos;
out vec4 vColor;
out vec3 vNormal;
out vec2 vTex;
out vec4 vExtra;

void main()
{
	vec4 pos = vec4(position[0], position[1], position[2], 1.0);

	vec3 dir = normal;
	dir.z = 0.;
	float len = length(dir);
	vec3 n = (len > 0.0001) ? dir / len : vec3(1., 0., 0.);

	mat2 turn = mat2(vec2(n.x, n.y), vec2(n.y, -n.x));
	turn[1] *= 0.02;
	vec2 offset = turn * vec2(0., tex.x);
	// x and y NDC units do not span equal physical screen distances
	// unless the window is square - without this, the same offset drawn
	// along a horizontal vs vertical bond looked a different physical
	// width. Same convention Image::rotateAspectCorrected() uses
	// (Window::aspect() == height/width, scaling x to compensate).
	offset.x *= aspect;
	pos.xy += offset;
	vTex.xy = tex.xy;
	vTex.x += 0.5;
	vPos = pos;
	gl_Position = pos;
	vColor = color;
	vExtra = extra;
	vNormal = n;
}
