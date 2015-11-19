#version 150

in vec4 vPosition;
in vec3 vColor;
uniform mat4 mModelView;
uniform mat4 mProjection;

out vec4 fColor;

void main()
{
    gl_Position = mProjection * mModelView * vPosition;
    fColor = vec4(vColor, 1.0);
}
