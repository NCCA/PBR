#version 330 core
// based on demo code from here https://learnopengl.com/#!Advanced-Lighting/Shadows/Point-Shadows

layout (location = 0) in vec3 inVert;

uniform mat4 M;

void main()
{
    gl_Position = M * vec4(inVert, 1.0);
}
