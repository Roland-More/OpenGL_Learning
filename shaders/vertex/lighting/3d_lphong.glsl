#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalModel;


void main()
{
    // for lighting
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalModel * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
