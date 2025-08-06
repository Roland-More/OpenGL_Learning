#version 330 core

struct Material {
    sampler2D diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform sampler2D shadowMap;


bool isInShadow(vec4 fragPosLightSpace, vec3 lightDir, vec3 norm)
{
    // perform perspective divide (not needed for orthographic proj.)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;

    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);

    return projCoords.z - bias > closestDepth;
}


void main()
{
    // General
    vec3 lightDir = normalize(-light.direction);
    vec3 norm = normalize(fs_in.Normal);

    // Ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));

    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    vec3 specular = vec3(0.0, 0.0, 0.0);

    // Dont calculate diffuse and specular if in shadow
    if (!isInShadow(fs_in.FragPosLightSpace, lightDir, norm))
    {
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords)); 

        // Specular
        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  

        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        specular = material.specular * spec * light.specular;
    }

    // Lighting Result
    vec3 result = ambient + diffuse + specular;

    // Gamma correction
    result = pow(result, vec3(1.0/2.2));

    FragColor = vec4(result, 1.0);
}
