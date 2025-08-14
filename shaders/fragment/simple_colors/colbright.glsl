#version 330 core

out vec4 FragColor;

uniform vec3 fColor;


float findHighest(vec3 color)
{
    float highest = color.r;

    if (color.g >= highest)
        highest = color.g;
    if (color.b >= highest)
        highest = color.b;

    return highest > 0.0 ? highest : 1.0;
}


void main()
{
    FragColor = vec4(fColor / findHighest(fColor), 1.0);
}
