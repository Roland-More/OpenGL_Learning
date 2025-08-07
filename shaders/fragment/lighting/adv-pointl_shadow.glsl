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
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

uniform sampler2D shadowMap;
uniform float near_plane;
uniform float far_plane;


float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

float shadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 norm)
{
    // perform perspective divide (not needed for orthographic proj.)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0)
        return 0.0;

    float currentDepth = LinearizeDepth(projCoords.z) / far_plane;
    float bias = max(0.03 * (1.0 - dot(norm, lightDir)), 0.003);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    return shadow;
}


void main()
{
    // General
    vec3 lightDir = normalize(light.position - fs_in.FragPos);
    vec3 norm = normalize(fs_in.Normal);
    float ldistance   = length(light.position - fs_in.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * ldistance + 
                            light.quadratic * ldistance * ldistance);

    // Ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords)) * attenuation;

    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    vec3 specular = vec3(0.0, 0.0, 0.0);

    // Dont calculate diffuse and specular if completely in shadow
    float shadow = shadowCalculation(fs_in.FragPosLightSpace, lightDir, norm);
    if (shadow != 1.0)
    {
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords)) * attenuation; 

        // Specular
        vec3 viewDir = normalize(viewPos - fs_in.FragPos);
        vec3 halfwayDir = normalize(viewDir + lightDir);  

        float spec = pow(max(dot(halfwayDir, norm), 0.0), material.shininess);
        specular = material.specular * spec * light.specular * attenuation;
    }

    // Lighting Result
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

    // Gama correction
    result = pow(result, vec3(1.0/2.2));

    FragColor = vec4(result, 1.0);
}
