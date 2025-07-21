#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture0;

void main()
{
    FragColor = vec4(0.8, 0.8, 0.8, 1.0) * texture(texture0, TexCoord);
}
