#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Se calcula la posición proyectada
    vec4 pos = projection * view * model * vec4(position, 1.0);
    
    // TRUCO DE PROFUNDIDAD: Forzar Z = W para que el fragmento quede en la máxima lejanía (profundidad 1.0)
    gl_Position = pos.xyww;
}