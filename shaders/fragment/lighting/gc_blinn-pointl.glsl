#version 330 core

struct Material {
    sampler2D diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;


void main()
{
    // General
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 norm = normalize(Normal);
    float ldistance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * ldistance + 
                            light.quadratic * ldistance * ldistance);

    // Ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords)) * attenuation;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords)) * attenuation; 

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 halfwayDir = normalize(viewDir + lightDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);

    vec3 specular = spec * light.specular * material.specular * attenuation;

    // Result
    vec3 result = ambient + diffuse + specular;

    // Gama correction
    result = pow(result, vec3(1.0/2.2));

    FragColor = vec4(result, 1.0);
}
