#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 texCoords;

uniform vec2 u_resolution;

void main() 
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    
    texCoords = aTexCoords;
}