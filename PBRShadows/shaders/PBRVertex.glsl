#version 410 core
// this demo is based on code from here https://learnopengl.com/#!PBR/Lighting
/// @brief the vertex passed in
layout (location = 0) in vec3 inVert;
/// @brief the normal passed in
layout (location = 1) in vec3 inNormal;
/// @brief the in uv
layout (location = 2) in vec2 inUV;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

layout( std140) uniform TransformUBO
{
  mat4 MVP;
  mat4 normalMatrix;
  mat4 M;
}transforms;


uniform mat2 textureRotation;
void main()
{

  // rotate texture cords for visual interest
  TexCoords=textureRotation*inUV;
  WorldPos = vec3(transforms.M * vec4(inVert, 1.0f));

  Normal=mat3(transforms.normalMatrix)*inNormal;
  gl_Position = transforms.MVP*vec4(inVert,1.0);
}
