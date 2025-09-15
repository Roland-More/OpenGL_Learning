// Simple data output debug shader example

#version 330 core
out vec4 FragColor;
in vec3 Normal;
[...]
  
void main()
{
    [...]
    FragColor.rgb = Normal;
    FragColor.a = 1.0f;
}
