#version 150

#ge_include "standard/lighting.fs"

out vec4 ge_fragmentColor;

void main()
{
	ge_fragmentColor = lighting(vec4(1.0));
}
