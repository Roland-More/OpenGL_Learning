#version 330 core

#define NUM_LIGHTS 4

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material {
    sampler2D diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 diffuse;
    vec3 specular;
};


in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform Material material;
uniform Light lights[NUM_LIGHTS];
uniform vec3 ambient;

uniform vec3 viewPos;


vec3 calculateLighting(Light light, vec3 norm, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    float ldistance   = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (ldistance * ldistance);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords)) * attenuation; 

    // Specular
    vec3 halfwayDir = normalize(viewDir + lightDir);

    float spec = pow(max(dot(halfwayDir, norm), 0.0), material.shininess);
    vec3 specular = material.specular * spec * light.specular * attenuation;

    return diffuse + specular;
} 


void main()
{
    // General
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    // Ambient
    vec3 ambient = ambient * vec3(texture(material.diffuse, fs_in.TexCoords));

    // Lights
    vec3 result = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < NUM_LIGHTS; i++)
        result += calculateLighting(lights[i], norm, viewDir);

    // Lighting Result
    result += ambient;

    FragColor = vec4(result, 1.0);

    // check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
