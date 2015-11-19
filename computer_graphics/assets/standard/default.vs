#version 150

in vec4 ge_position;
in vec3 ge_normal;
in vec2 ge_textureCoordinates;

uniform mat4 ge_modelViewProjection;
uniform mat4 ge_modelView;
uniform mat3 ge_normalMatrix;

uniform mat4 ge_modelViewProjectionLight1;
uniform mat4 ge_modelViewProjectionLight2;

out vec2 textureCoordinates;
out vec3 normal;
out vec3 position;
out vec4 positionLight1;
out vec4 positionLight2;

void main()
{
	gl_Position = ge_modelViewProjection * ge_position;

	textureCoordinates = ge_textureCoordinates;
	normal = normalize(ge_normalMatrix * ge_normal);
	position = (ge_modelView * ge_position).xyz;
	positionLight1 = ge_modelViewProjectionLight1 * ge_position;
	positionLight2 = ge_modelViewProjectionLight2 * ge_position;
}
