#version 330 core

in vec3 Result;

out vec4 FragColor;

void main()
{
    FragColor = vec4(Result, 1.0);
}
