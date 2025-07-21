#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D texture0;
uniform vec3 colorFilter;

void main()
{
    vec4 base = texture(texture0, TexCoord);
    base.y = base.x;
    base.z = base.x;
    FragColor = vec4(colorFilter, 1.0) * base;
}
