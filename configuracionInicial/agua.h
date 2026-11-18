#pragma once
/*
	IMPORTANTE: incluye glad (o glew) y glfw ANTES de este archivo.
	Funciona con los dos tipos de Shader:
	  - shader_m.h (GLAD)   -> usa shader.ID
	  - Shader.h   (GLEW)   -> usa shader.Program

	AguaAnimada:
	  + draw()        -> dibuja una malla plana propia (estanque generado por código)
	  + drawModelo()  -> dibuja TU modelo (agua.obj) con el shader de olas
	  + preparar() / terminar() -> por si quieres controlar el dibujo tú mismo
	  + moverConTeclas() -> acomodar el agua mientras corre el programa
*/
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

namespace aguaDetalle
{
	// Obtiene el id del programa sin importar cómo se llame en tu clase Shader
	template <typename S>
	auto idPrograma(S& s, int) -> decltype(static_cast<unsigned int>(s.ID)) { return static_cast<unsigned int>(s.ID); }
	template <typename S>
	auto idPrograma(S& s, long) -> decltype(static_cast<unsigned int>(s.Program)) { return static_cast<unsigned int>(s.Program); }

	inline void setFloat(unsigned int p, const char* n, float v) { glUniform1f(glGetUniformLocation(p, n), v); }
	inline void setInt(unsigned int p, const char* n, int v) { glUniform1i(glGetUniformLocation(p, n), v); }
	inline void setVec2(unsigned int p, const char* n, const glm::vec2& v) { glUniform2f(glGetUniformLocation(p, n), v.x, v.y); }
	inline void setVec3(unsigned int p, const char* n, const glm::vec3& v) { glUniform3f(glGetUniformLocation(p, n), v.x, v.y, v.z); }
	inline void setMat4(unsigned int p, const char* n, const glm::mat4& m) { glUniformMatrix4fv(glGetUniformLocation(p, n), 1, GL_FALSE, glm::value_ptr(m)); }
}

class AguaAnimada
{
public:
	unsigned int VAO = 0, VBO = 0, EBO = 0;
	unsigned int indexCount = 0;

	// ---- Parámetros que puedes cambiar ----
	float amplitud = 0.5f;		// altura de las olas
	float frecuencia = 0.6f;	// qué tan juntas están las olas (más chico = olas más anchas)
	float velocidad = 2.0f;		// qué tan rápido se mueven
	glm::vec3 colorAgua = glm::vec3(0.05f, 0.35f, 0.55f);
	float transparencia = 0.85f;	// 1.0 = opaca
	float brilloCrestas = 0.12f;	// manchas claras en las crestas de las olas (0 = ninguna, 0.6 = muy blancas)
	float brilloEspecular = 0.25f;	// destellos de luz sobre el agua (0 = sin destellos)
	float brilloFresnel = 0.25f;	// velo claro a ras de agua y a distancia (0 = sin velo)
	glm::vec3 luzDir = glm::vec3(-0.5f, -0.7f, -0.5f);
	glm::vec3 luzColor = glm::vec3(1.0f, 0.97f, 0.9f);

	// Textura opcional propia (0 = sin textura). Debe estar cargada con GL_REPEAT.
	unsigned int textura = 0;
	// Poner en true si dibujas un Model que ya trae su textura (agua.obj)
	bool texturaDelModelo = false;
	float tiling = 6.0f;
	glm::vec2 scrollTextura = glm::vec2(0.03f, 0.015f);

	// Crea agua.vs y agua.fs dentro de la carpeta que le indiques (por ejemplo "Shader").
	// Así no tienes que copiar los archivos a mano: se escriben exactamente donde tu programa los busca.
	// Llamar ANTES de crear el Shader del agua.
	static bool crearArchivosShader(const std::string& carpeta)
	{
		const char* vs = R"GLSL(#version 330 core
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
)GLSL";
		const char* fs = R"GLSL(#version 330 core
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
)GLSL";

		std::string rutaVS = carpeta + "/agua.vs";
		std::string rutaFS = carpeta + "/agua.fs";

		std::ofstream archivoVS(rutaVS.c_str(), std::ios::out | std::ios::trunc);
		if (!archivoVS.is_open())
		{
			std::cout << "AGUA: no se pudo escribir " << rutaVS << " (revisa que exista la carpeta '" << carpeta << "')" << std::endl;
			return false;
		}
		archivoVS << vs;
		archivoVS.close();

		std::ofstream archivoFS(rutaFS.c_str(), std::ios::out | std::ios::trunc);
		if (!archivoFS.is_open())
		{
			std::cout << "AGUA: no se pudo escribir " << rutaFS << std::endl;
			return false;
		}
		archivoFS << fs;
		archivoFS.close();

		std::cout << "AGUA: shaders listos en " << carpeta << "/" << std::endl;
		return true;
	}

	// size = largo/ancho total del estanque, divisiones = celdas por lado
	void init(float size = 24.0f, int divisiones = 120)
	{
		std::vector<float> v;
		std::vector<unsigned int> idx;
		float mitad = size * 0.5f;

		for (int z = 0; z <= divisiones; z++)
		{
			for (int x = 0; x <= divisiones; x++)
			{
				float u = (float)x / divisiones;
				float w = (float)z / divisiones;
				// posición
				v.push_back(-mitad + size * u);
				v.push_back(0.0f);
				v.push_back(-mitad + size * w);
				// normal (hacia arriba)
				v.push_back(0.0f);
				v.push_back(1.0f);
				v.push_back(0.0f);
				// coordenadas de textura
				v.push_back(u);
				v.push_back(w);
			}
		}

		for (int z = 0; z < divisiones; z++)
		{
			for (int x = 0; x < divisiones; x++)
			{
				unsigned int i0 = z * (divisiones + 1) + x;
				unsigned int i1 = i0 + 1;
				unsigned int i2 = i0 + (divisiones + 1);
				unsigned int i3 = i2 + 1;
				idx.push_back(i0); idx.push_back(i2); idx.push_back(i1);
				idx.push_back(i1); idx.push_back(i2); idx.push_back(i3);
			}
		}
		indexCount = (unsigned int)idx.size();

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

		// layout igual al de los Model: 0 = posición, 1 = normal, 2 = texcoords
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}

	// Crea el agua a partir de TRIÁNGULOS en el plano XZ, a una altura 'y' (coordenadas de mundo:
	// se dibuja con matriz identidad). Sirve para que el agua tenga la forma EXACTA de un foso o estanque.
	//   tris        = lista de puntos, de 3 en 3 (cada 3 puntos = un triángulo; x = X, y = Z)
	//   divisiones  = en cuántas partes se divide cada lado de cada triángulo (más = olas más suaves)
	//   uvPorUnidad = repeticiones de textura por unidad de escena (solo si usas textura)
	void initTriangulos(const std::vector<glm::vec2>& tris, float y, int divisiones = 40, float uvPorUnidad = 0.1f)
	{
		std::vector<float> v;
		std::vector<unsigned int> idx;
		int n = divisiones;

		for (size_t t = 0; t + 2 < tris.size(); t += 3)
		{
			glm::vec2 a = tris[t], b = tris[t + 1], c = tris[t + 2];
			// que la cara mire hacia arriba (+Y)
			float cy = (b.y - a.y) * (c.x - a.x) - (b.x - a.x) * (c.y - a.y);
			if (cy < 0.0f) std::swap(b, c);

			unsigned int base = (unsigned int)(v.size() / 8);

			// vértices (i, j) con i + j <= n
			for (int i = 0; i <= n; i++)
			{
				for (int j = 0; j <= n - i; j++)
				{
					glm::vec2 p = a + (b - a) * ((float)i / (float)n) + (c - a) * ((float)j / (float)n);
					v.push_back(p.x); v.push_back(y); v.push_back(p.y);		// posición
					v.push_back(0.0f); v.push_back(1.0f); v.push_back(0.0f);	// normal (arriba)
					v.push_back(p.x * uvPorUnidad); v.push_back(p.y * uvPorUnidad);	// texcoords
				}
			}

			// índice del vértice (i, j) dentro de este triángulo
			auto id = [&](int i, int j) -> unsigned int
			{
				return base + (unsigned int)(i * (n + 1) - (i * (i - 1)) / 2 + j);
			};

			for (int i = 0; i < n; i++)
			{
				for (int j = 0; j < n - i; j++)
				{
					idx.push_back(id(i, j)); idx.push_back(id(i + 1, j)); idx.push_back(id(i, j + 1));
					if (j < n - i - 1)
					{
						idx.push_back(id(i + 1, j)); idx.push_back(id(i + 1, j + 1)); idx.push_back(id(i, j + 1));
					}
				}
			}
		}
		indexCount = (unsigned int)idx.size();

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
	}

	// Manda todos los uniforms al shader del agua y activa la transparencia.
	// (el "model" lo manda quien dibuja)
	template <typename S>
	void preparar(S& shader, const glm::mat4& view, const glm::mat4& projection,
		const glm::vec3& viewPos, float tiempo)
	{
		using namespace aguaDetalle;
		unsigned int p = idPrograma(shader, 0);
		glUseProgram(p);

		setMat4(p, "view", view);
		setMat4(p, "projection", projection);
		setVec3(p, "viewPos", viewPos);

		setFloat(p, "uTime", tiempo);
		setFloat(p, "waveAmplitude", amplitud);
		setFloat(p, "waveFrequency", frecuencia);
		setFloat(p, "waveSpeed", velocidad);

		setVec3(p, "lightDir", luzDir);
		setVec3(p, "lightColor", luzColor);
		setVec3(p, "colorAgua", colorAgua);
		setFloat(p, "alpha", transparencia);
		setFloat(p, "brilloCrestas", brilloCrestas);
		setFloat(p, "brilloEspecular", brilloEspecular);
		setFloat(p, "brilloFresnel", brilloFresnel);

		setVec2(p, "waterSpeed", scrollTextura);
		setFloat(p, "tiling", tiling);
		setInt(p, "useTexture", (textura != 0 || texturaDelModelo) ? 1 : 0);
		if (textura != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textura);
			setInt(p, "texture_diffuse1", 0);
		}

		// Recuerda si el blend ya estaba activo para dejarlo como estaba
		blendPrevio = (glIsEnabled(GL_BLEND) == GL_TRUE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void terminar()
	{
		if (!blendPrevio) glDisable(GL_BLEND);
	}

	// Dibuja la malla propia (el estanque generado por código)
	template <typename S>
	void draw(S& shader, const glm::mat4& model, const glm::mat4& view,
		const glm::mat4& projection, const glm::vec3& viewPos, float tiempo)
	{
		preparar(shader, view, projection, viewPos, tiempo);
		aguaDetalle::setMat4(aguaDetalle::idPrograma(shader, 0), "model", model);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		terminar();
	}

	// Dibuja TU modelo (por ejemplo agua.obj) con el shader de olas
	template <typename S, typename M>
	void drawModelo(S& shader, M& modelo, const glm::mat4& model, const glm::mat4& view,
		const glm::mat4& projection, const glm::vec3& viewPos, float tiempo)
	{
		preparar(shader, view, projection, viewPos, tiempo);
		aguaDetalle::setMat4(aguaDetalle::idPrograma(shader, 0), "model", model);
		modelo.Draw(shader);
		terminar();
	}

	// Flechas = mover en X/Z,  RePág / AvPág = subir / bajar.  Imprime la posición en consola.
	// Llamar desde el callback de teclado.
	bool moverConTeclas(int key, int action, glm::vec3& pos)
	{
		if (action == GLFW_RELEASE) return false;

		if (key == GLFW_KEY_UP)             pos.z -= 1.0f;
		else if (key == GLFW_KEY_DOWN)      pos.z += 1.0f;
		else if (key == GLFW_KEY_LEFT)      pos.x -= 1.0f;
		else if (key == GLFW_KEY_RIGHT)     pos.x += 1.0f;
		else if (key == GLFW_KEY_PAGE_UP)   pos.y += 0.25f;
		else if (key == GLFW_KEY_PAGE_DOWN) pos.y -= 0.25f;
		else return false;

		std::cout << "posAgua = (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
		return true;
	}

private:
	bool blendPrevio = false;
};
