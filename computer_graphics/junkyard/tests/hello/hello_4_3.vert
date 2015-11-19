#version 430 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) uniform mat2 mTransform;

void main()
{
    gl_Position = mat4(mTransform) * vPosition;
}
