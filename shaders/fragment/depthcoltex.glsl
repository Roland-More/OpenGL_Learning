#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture0;


void main()
{             
    float depth = texture(texture0, TexCoord).r;
    FragColor = vec4(vec3(depth), 1.0);
}
