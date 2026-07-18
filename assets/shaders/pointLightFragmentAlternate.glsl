#version 460 core
in vec2 Pos;
out vec4 FragColor;

uniform float shadowFalloff;

layout(binding = 5) uniform sampler2D occlusionMap; 

layout(std430, binding = 0) buffer LightData {
    float lightData[];
};
uniform int lightCount;

// Return Value: 0 is lit, rayProgress otherwise.
float rayMarch(vec2 rayOrigin, float rayAngle, vec2 rayDest) {
    float rayProgress;
    for (rayProgress = 5.0f; rayProgress < 1000; rayProgress++) {
        if (rayProgress > distance(rayOrigin, rayDest)) {
            return 0.0f;
        }
        vec2 occlusionTexCoords = vec2((rayOrigin.x + (rayProgress * cos(rayAngle)))/1280.0f, 1.0f - ((rayOrigin.y + (rayProgress * sin(rayAngle)))/720.0f));
        vec4 occlusionCheck = texture(occlusionMap, occlusionTexCoords);
        if (occlusionCheck.x > 0.5f) {
            return rayProgress;
        }
    }
    return 0.0f;
}

void main()
{
    vec4 midColor = {0.07f, 0.07f, 0.07f, 1.0f};
    for (int i = 0; i < lightCount * 8; i += 8) {
        vec2 center = { lightData[i], lightData[i+1] };
        float radius = lightData[i+2];
        float brightness = lightData[i+3];
        float falloff = lightData[i+4];
        vec3 lightColor = { lightData[i+5], lightData[i+6], lightData[i+7] };

        float angle = atan((center.y - Pos.y), (center.x - Pos.x));
        float result = rayMarch(Pos, angle, center);

        float dist = distance(Pos, center);
        float normDist = dist / radius;

        // Simple Method
        
        float attenuation = max(0.0, 1.0 - (dist/radius));
        attenuation = pow(attenuation, falloff);
        
        // Inverse Square?
        /*
        float attenuation = 1.0 / (normDist * normDist * 50 + 0.002);
        float window = clamp(1.0 - pow(normDist, falloff), 0.0, 1.0);
        attenuation *= window;
        */
        vec4 litColor = vec4(lightColor * brightness * attenuation, 1.0);
        vec4 shadowColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

        if (result < 0.1f) {
            midColor = max(litColor, midColor);
        } else {
            if (result < dist) {
                midColor = max(mix(shadowColor, litColor, pow(result/dist, shadowFalloff)), midColor);
            }
        }
    }
    FragColor = midColor;
}