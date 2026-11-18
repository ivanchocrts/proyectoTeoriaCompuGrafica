#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out float Altura;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float uTime;
uniform float waveAmplitude;
uniform float waveFrequency;
uniform float waveSpeed;

void main()
{
	vec3 pos = aPos;

	// Dos olas con direcciones y velocidades distintas
	float fase1 = pos.x * waveFrequency + uTime * waveSpeed;
	float fase2 = pos.z * waveFrequency * 0.8 + uTime * waveSpeed * 1.3;

	float h = (sin(fase1) + sin(fase2)) * 0.5;	// -1 .. 1
	pos.y += h * 2.0 * waveAmplitude;
	Altura = h;

	// Normal calculada con la derivada de las olas (para que la luz "brille" en las crestas)
	float dhdx = cos(fase1) * waveFrequency * waveAmplitude;
	float dhdz = cos(fase2) * waveFrequency * 0.8 * waveAmplitude;
	vec3 normalOla = normalize(vec3(-dhdx, 1.0, -dhdz));

	FragPos = vec3(model * vec4(pos, 1.0));
	Normal = mat3(transpose(inverse(model))) * normalOla;
	TexCoords = aTexCoords;

	gl_Position = projection * view * vec4(FragPos, 1.0);
}
