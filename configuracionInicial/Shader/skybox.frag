#version 330 core
out vec4 color;

uniform vec3 starColor;

void main()
{
    // Emite color directo sin atenuación ni sombras
    color = vec4(starColor, 1.0);
}