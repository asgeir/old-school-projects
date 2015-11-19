#version 430 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;
layout(location = 0) uniform mat4 mModelView;
layout(location = 1) uniform mat4 mProjection;

out vec4 fColor;

void main()
{
    gl_Position = mProjection * mModelView * vPosition;
    fColor = vec4(vColor, 1.0);
}
