#version 150

in vec4 vPosition;
uniform mat2 mTransform;

void main()
{
    gl_Position = mat4(mTransform) * vPosition;
}
