#version 330 core

#define NUM_LIGHTS 4

out vec4 FragColor;


struct Light {
    vec3 position;

    vec3 diffuse;
    vec3 specular;
};


in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform float shininess;
uniform vec3 ambient;

uniform Light lights[NUM_LIGHTS];

uniform vec3 viewPos;

uniform float exposure;


vec3 calculateLighting(Light light, vec3 Normal, vec3 viewDir, vec3 FragPos, vec3 Albedo, float Specular)
{
    vec3 lightDir = normalize(light.position - FragPos);
    float ldistance   = length(light.position - FragPos);
    float attenuation = 1.0 / (ldistance * ldistance);

    // Diffuse
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * Albedo * attenuation; 

    // Specular
    vec3 halfwayDir = normalize(viewDir + lightDir);

    float spec = pow(max(dot(halfwayDir, Normal), 0.0), shininess);
    vec3 specular = vec3(Specular) * spec * light.specular * attenuation;

    return diffuse + specular;
}


void main()
{
    // Retrieve G-buffer values
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // General
    vec3 viewDir = normalize(viewPos - FragPos);

    // Ambient
    vec3 ambient = ambient * Albedo;

    // Lights
    vec3 result = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < NUM_LIGHTS; i++)
        result += calculateLighting(lights[i], Normal, viewDir, FragPos, Albedo, Specular);

    // Lighting Result
    result += ambient;

    // gamma correction + hdr
    const float gamma = 2.2;
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-result * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
}
