#version 330 core
// based on demo code from here https://learnopengl.com/#!Advanced-Lighting/Shadows/Point-Shadows
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;
uniform bool lightActive;
void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    if(lightActive == true)
    // write this as modified depth
      gl_FragDepth = lightDistance;
    else
      gl_FragDepth = 1.0f;
}
