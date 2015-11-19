#version 150

#define MAX_ITER 3

in vec2 textureCoordinates;

uniform float time;

out vec4 ge_fragmentColor;

// http://glsl.heroku.com/e#11051.0
void main()
{
	vec2 p = textureCoordinates*2.0*3.14159265- vec2(20.0);
	vec2 i = p;
	float c = 1.0;
	float inten = .05;

	for (int n = 0; n < MAX_ITER; n++)
	{
		float t = time * (1.0 - (3.0 / float(n+1)));
		i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0/length(vec2(p.x / (sin(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
	}
	c /= float(MAX_ITER);
	c = 1.5-sqrt(c);

	vec3 modulatedColor = vec3(4.0, 0.0, 0.0) * vec3(c*c*c*c);
	ge_fragmentColor = vec4(min(modulatedColor, vec3(1.0)), 1.0);
	//ge_fragmentColor = vec4(vec3(c*c*c*c), 1.0);
}
