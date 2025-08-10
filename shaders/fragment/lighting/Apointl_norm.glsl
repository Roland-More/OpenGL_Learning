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

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
} fs_in;

out vec4 FragColor;

uniform Material material;
uniform Light light;

uniform vec3 viewPos;

uniform sampler2D normalMap;


void main()
{
    // General
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    vec3 norm = texture(normalMap, fs_in.TexCoords).rgb;
    norm = normalize(norm * 2 - 1.0);

    float ldistance   = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * ldistance + 
                            light.quadratic * ldistance * ldistance);

    // Ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords)) * attenuation;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords)) * attenuation; 

    // Specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(viewDir + lightDir);  

    float spec = pow(max(dot(halfwayDir, norm), 0.0), material.shininess);
    vec3 specular = material.specular * spec * light.specular * attenuation;
    
    // Lighting Result
    vec3 result = ambient + diffuse + specular;

    // Gama correction
    result = pow(result, vec3(1.0/2.2));

    FragColor = vec4(result, 1.0);
}
