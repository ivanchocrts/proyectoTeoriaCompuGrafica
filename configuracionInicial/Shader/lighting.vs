#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Uniformes para animación de agua
uniform int isWater = 0;              // 1 = Activa animación en el objeto, 0 = Estático
uniform float uTime = 0.0;            // Tiempo transcurrido enviado desde C++
uniform float waveAmplitude = 0.03;   // Altura de la ola
uniform float waveFrequency = 1.5;    // Frecuencia espacial
uniform float waveSpeed = 1.8;        // Velocidad de movimiento de la ola

void main()
{
    vec3 localPos = position;
    vec3 localNormal = normal;

    if (isWater == 1)
    {
        // Oscilación sinusoidal en Y
        float wave = sin(position.x * waveFrequency + uTime * waveSpeed) 
                   + cos(position.z * waveFrequency * 0.8 + uTime * waveSpeed * 1.2);
        
        localPos.y += wave * waveAmplitude;

        // Inclinación suave de la normal para simular reacción a la luz
        localNormal.x -= cos(position.x * waveFrequency + uTime * waveSpeed) * waveAmplitude * waveFrequency;
        localNormal.z -= -sin(position.z * waveFrequency * 0.8 + uTime * waveSpeed * 1.2) * waveAmplitude * waveFrequency;
        localNormal = normalize(localNormal);
    }

    FragPos = vec3(model * vec4(localPos, 1.0f));
    Normal = mat3(transpose(inverse(model))) * localNormal;
    TexCoords = texCoords;

    gl_Position = projection * view * model * vec4(localPos, 1.0f);
}