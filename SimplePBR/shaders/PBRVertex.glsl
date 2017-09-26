#version 410 core
/*
layout (location = 0) in vec3 pos;
layout (location = 2) in vec3 normal;
layout (location = 1) in vec2 texCoords;
*/
/// @brief the vertex passed in
layout (location = 0) in vec3 inVert;
/// @brief the normal passed in
layout (location = 2) in vec3 inNormal;
/// @brief the in uv
layout (location = 1) in vec2 inUV;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
/*
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    TexCoords = texCoords;
    WorldPos = vec3(model * vec4(pos, 1.0f));
    Normal = mat3(model) * normal;

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}
*/

uniform mat4 MVP;
uniform mat3 normalMatrix;
uniform mat4 M;

void main()
{
  WorldPos = vec3(M * vec4(inVert, 1.0f));
  Normal=normalMatrix*inNormal;
  gl_Position = MVP*vec4(inVert,1.0);

}
