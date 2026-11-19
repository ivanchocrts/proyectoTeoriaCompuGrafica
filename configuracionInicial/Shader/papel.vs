#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;
uniform float windStrength;
uniform float waveSpeed;

void main()
{
    TexCoords = aTexCoords;
    
    // ANCLA: 0 = no se mueve (hilo), 1 = se mueve al máximo (borde del papel).
    // Si el papel se mueve de arriba en lugar de abajo, cambia esta línea a: float ancla = 1.0 - TexCoords.y;
    float ancla = TexCoords.y; 
    
    vec3 animatedPos = aPos;
    
    // Ondeo súper suave. aPos.x * 0.5 le da una asimetría natural sin arrugarlo.
    float wave = sin(time * waveSpeed + aPos.x * 0.5);
    
    // Balanceamos el papel en el eje Z (adelante/atrás) como si el viento lo empujara
    animatedPos.z += wave * windStrength * ancla;

    gl_Position = projection * view * model * vec4(animatedPos, 1.0);
}