#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

layout(binding = 0) uniform sampler2D albedoTextureID;
layout(binding = 1) uniform sampler2D ambientTextureID;

void main()
{
    vec4 albedoColor = texture(albedoTextureID, TexCoords);
    vec4 lightColor = texture(ambientTextureID, TexCoords);
    
    FragColor = albedoColor * lightColor;
}