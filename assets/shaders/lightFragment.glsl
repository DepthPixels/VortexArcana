#version 460 core
in vec2 Pos;
out vec4 FragColor;

uniform vec2 center;
uniform float radius;
uniform float brightness;
uniform float falloff;
uniform vec3 lightColor;

void main()
{
    float dist = distance(Pos, center);

    float attenuation = max(0.0, 1.0 - (dist/radius));
    attenuation = pow(attenuation, falloff);

    FragColor = vec4(lightColor * brightness * attenuation, 1.0);
}