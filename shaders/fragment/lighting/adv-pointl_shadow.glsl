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
} fs_in;

out vec4 FragColor;

uniform Material material;
uniform Light light;

uniform vec3 viewPos;

uniform samplerCube shadowMap;
uniform float far_plane;


const vec3 shadowCalculationSampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);


float shadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - light.position;

    float currentDepth = length(fragToLight);

    float shadow  = 0.0;
    float bias    = 0.15;
    int samples   = 20;
    float offset  = 0.1;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;  

    for (int i = 0; i < samples; i++)
    {
        float closestDepth = texture(shadowMap, fragToLight + shadowCalculationSampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane; // undo mapping [0;1]
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);

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

    // Dont calculate diffuse and specular if completely in shadow
    vec3 diffuse = vec3(0.0, 0.0, 0.0);
    vec3 specular = vec3(0.0, 0.0, 0.0);

    float shadow = shadowCalculation(fs_in.FragPos);
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
