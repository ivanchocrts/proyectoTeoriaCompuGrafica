#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in float Altura;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 colorAgua;
uniform float alpha;

uniform float uTime;
uniform vec2 waterSpeed;
uniform float tiling;
uniform int useTexture;
uniform sampler2D texture_diffuse1;

uniform float brilloCrestas;
uniform float brilloEspecular;
uniform float brilloFresnel;

void main()
{
	vec3 base = colorAgua;

	// Si hay textura, se desplaza con el tiempo para que parezca que el agua fluye
	if (useTexture == 1)
	{
		vec2 uv = TexCoords * tiling + waterSpeed * uTime;
		base = mix(colorAgua, texture(texture_diffuse1, uv).rgb, 0.6);
	}

	// Las crestas de las olas se ven más claras (hace que el movimiento se note mucho)
	float cresta = smoothstep(0.35, 1.0, Altura);
	base = mix(base, vec3(0.45, 0.65, 0.80), cresta * brilloCrestas);

	// Patrón que se mueve con el tiempo (da sensación de corriente aunque no haya textura)
	float patron = sin(FragPos.x * 1.3 + uTime * 2.0) * sin(FragPos.z * 1.1 - uTime * 1.6);
	base = clamp(base + patron * 0.03, 0.0, 1.0);

	vec3 N = normalize(Normal);
	vec3 L = normalize(-lightDir);
	vec3 V = normalize(viewPos - FragPos);
	vec3 R = reflect(-L, N);

	float diff = max(dot(N, L), 0.0);
	float spec = pow(max(dot(V, R), 0.0), 64.0);
	float fresnel = pow(1.0 - max(dot(N, V), 0.0), 3.0);

	vec3 ambient = 0.35 * base;
	vec3 resultado = ambient
					+ diff * base * lightColor
					+ spec * lightColor * brilloEspecular
					+ fresnel * vec3(0.15, 0.25, 0.35) * brilloFresnel;

	FragColor = vec4(resultado, alpha);
}
