#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    TexCoords = aTexCoords;
    vec3 pos = aPos;
    
    // Semilla basada en la posicion X y Z para que cada vela se mueva a diferente ritmo
    float desfaseVela = aPos.x * 3.0 + aPos.z * 3.0;
    
    // Onda que sube verticalmente por el cuerpo de la flama (aPos.y)
    float ondaFuego = aPos.y * 10.0 - time * 6.0 + desfaseVela;
    
    // Movimiento sutil de serpenteo lateral (X, Z) y parpadeo vertical (Y)
    pos.x += sin(ondaFuego) * 0.025;
    pos.z += cos(ondaFuego * 0.8) * 0.025;
    pos.y += sin(time * 10.0 + desfaseVela) * 0.015;

    gl_Position = projection * view * model * vec4(pos, 1.0);
}