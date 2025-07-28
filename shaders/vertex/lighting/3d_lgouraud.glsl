#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Result;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalModel;

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;


void main()
{
    // General lighting
    vec3 Normal = normalModel * aNormal;
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 norm = normalize(Normal);

    // Ambient
    vec3 ambient = 0.1 * lightColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 1.0;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // Result
    Result = (ambient + diffuse + specular) * objectColor;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
