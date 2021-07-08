#version 330 core
in vec3 fsNormal;
in vec3 fsPosition;
in vec4 fsLightSpacePosition;

out vec4 FragColor;

uniform sampler2D depthMap;
uniform sampler2D normalMap;
uniform sampler2D worldPosMap;
uniform sampler2D fluxMap;
uniform sampler2D randomMap;

uniform float shadowBias;
uniform int shadowNum;
uniform float shadowRadius;
uniform vec3 viewPos;

uniform float indirectWeight;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
uniform Light light;

float shadowCalculation(vec3 projCoords) {
    // get closest depth value from light's perspective
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // calculate bias
    vec3 normal = normalize(fsNormal);
    vec3 lightDir = normalize(light.position - fsPosition);
    float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), 0.00005);

    float shadow = currentDepth - bias  > closestDepth  ? 1.0 : 0.0;

    // PCF
    // float shadow = 0.0;
    // vec2 texelSize = 1.0 / textureSize(depthMap, 0);
    // for (int x = -1; x <= 1; x++) {
    //     for (int y = -1; y <= 1; y++) {
    //         float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
    //         shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
    //     }    
    // }
    // shadow /= 9.0;
    // // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    // if(projCoords.z > 1.0)
    //     shadow = 0.0;
        
    return shadow;
}

void main() {
    // calculate coordinate in light space
    vec3 projCoords = fsLightSpacePosition.xyz / fsLightSpacePosition.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // calculate shadow
    float shadow = shadowCalculation(projCoords);

    // RSM
    vec3 indirect = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < shadowNum; i++) {
        vec3 r = texelFetch(randomMap, ivec2(i, 0), 0).xyz;
        vec2 sample_coord = projCoords.xy + r.xy * shadowRadius;
        float weight = r.z;

        vec3 target_normal = normalize(texture(normalMap, sample_coord).xyz);
        vec3 target_worldPos = texture(worldPosMap, sample_coord).xyz;
        vec3 target_flux = texture(fluxMap, sample_coord).rgb;

        vec3 indirect_result = target_flux * max(0, dot(target_normal, fsPosition - target_worldPos)) * max(0, dot(fsNormal, target_worldPos - fsPosition)) / pow(length(fsPosition - target_worldPos), 4.0);
        indirect_result *= weight;
        indirect += indirect_result;
    }
    indirect = clamp(indirect / shadowNum, 0.0, 1.0);

    vec3 lightDir = normalize(light.position - fsPosition);

    // ambient
    vec3 ambient = light.ambient * material.ambient;
    
    // diffuse
    vec3 norm = normalize(fsNormal);
    float diff = max(dot(norm, lightDir), 0.0);  
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    // specular
    vec3 viewDir=normalize(viewPos - fsPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;


    // attenuation
    // (we don't do that in small scene)
    // float dist = length(light.position-fsPosition);
    // float attenuation = min(1.0, 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist));
    // ambient *= attenuation;
    // diffuse *= attenuation;
    // specular *= attenuation;
    
    vec3 result = ambient + (diffuse + specular) * (1.0 - shadow) + indirect * indirectWeight;

    // gamma correction
    float gamma = 2.2;
    FragColor = vec4(result, 1.0);
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gamma));
}