#version 150

in vec4 ge_position;
in vec3 ge_normal;
in vec2 ge_textureCoordinates;

uniform mat4 ge_modelViewProjection;
uniform mat4 ge_modelView;
uniform mat3 ge_normalMatrix;

out vec2 textureCoordinates;
out vec3 normal;
out vec3 position;

void main()
{
	gl_Position = ge_modelViewProjection * ge_position;

	textureCoordinates = ge_textureCoordinates;
	normal = normalize(ge_normalMatrix * ge_normal);
	position = (ge_modelView * ge_position).xyz;
}
