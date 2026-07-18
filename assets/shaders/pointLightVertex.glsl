#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 Pos;

void main() 
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    
    Pos.x = aTexCoords.x * 1280.0f;
    Pos.y = (1.0f - aTexCoords.y) * 720.0f;
}