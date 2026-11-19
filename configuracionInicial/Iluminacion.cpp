// Iluminacion.cpp
#include <iostream>
#include <string>
#include <cmath>
#include <vector>

// GLEW
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "SOIL2/SOIL2.h"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_easy_font.h"
#include <vector>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"

using namespace std;

// Dimensiones de ventana
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Prototipos
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Cmara
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
bool keys[1024];
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool firstMouse = true;

//Audio
ma_engine audioEngine;
ma_sound musicaFondo;
bool musicaActiva = true;

bool person = false;
float cameraDistance = 8.0f;
float cameraHeight = 2.3f;
float cameraYaw = 0.0f;
float cameraPitch = 10.0f;

float mouseSensitivity = 0.4f;

float giroPerson = 180.0f,
personX = 0.0f,
personY = 1.0f,
personZ = 0.0f;

float rotBrazoDer = 0.0f,
rotBrazoIzq = 0.0f,
rotPiernaDer = 0.0f,
rotPiernaIzq = 0.0f,
rotCabezaSi = 0.0f,
rotCabezaNo = 0.0f;

bool mov = false;
float tiempoCaminata = 0.0f;
float anguloMaximo = 80.0f;
float anguloCaminata = 0.0f;
float velocidadCaminata = 40.0f;
float direccionCaminata = 1.0f;

//Para las vistas pedidas
bool verEscen = false;
bool verAnim = false;
bool verModel = false;

bool animTraja = false;
int etapaTraja = 0;
float trajaX = -8.0f;      
float trajaZ = -20.0f;     
float trajaRotY = 20.0f;   

// Luz direccional (sol fijo, ya sin animaci�n)
glm::vec3 dirLightDir(-0.3f, -1.0f, -0.2f); // direcci�n desde donde viene la luz

// DeltaTime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Cubo unitario para las l�mparas (por si luego quieres volver a usar lampShader)
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

int main()
{
    glfwInit();

    // Obtener resolucin del monitor
    const GLFWvidmode* modo = glfwGetVideoMode(glfwGetPrimaryMonitor());

    SCREEN_WIDTH = modo->width;
    SCREEN_HEIGHT = modo->height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    //GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto final. Computacion grafica", nullptr, nullptr);

    // Crear ventana con resolucin del monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Proyecto final. Computacion grafica", primaryMonitor, nullptr);

    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Audio
    if (ma_engine_init(NULL, &audioEngine) != MA_SUCCESS)
        std::cout << "No se pudo iniciar el audio" << std::endl;

    if (ma_sound_init_from_file(&audioEngine, "Audio/pista1.mp3",
        MA_SOUND_FLAG_STREAM, NULL, NULL, &musicaFondo) != MA_SUCCESS)
        std::cout << "No se encontro Audio/pista1.mp3" << std::endl;
    else
    {
        ma_sound_set_looping(&musicaFondo, MA_TRUE);  
        ma_sound_set_volume(&musicaFondo, 0.5f);      
        ma_sound_start(&musicaFondo);
    }

    // Shaders
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");
    Shader skyboxShader("Shader/SkyBox.vs", "Shader/SkyBox.frag");

    // ============================================================
    // SKYBOX: v�rtices del cubo (dentro de main)
    // ============================================================
    GLfloat skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    GLuint skyboxVBO, skyboxVAO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glBindVertexArray(0);

    // ============================================================
    // SKYBOX: carga del cubemap
    // ============================================================
    vector<const GLchar*> faces;
    faces.push_back("SkyBox/right.jpg");
    faces.push_back("SkyBox/left.jpg");
    faces.push_back("SkyBox/top.jpg");
    faces.push_back("SkyBox/bottom.jpg");
    faces.push_back("SkyBox/back.jpg");
    faces.push_back("SkyBox/front.jpg");

    GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

    // ============================================================
    // Modelos
    // ============================================================
    Model escena_principal((char*)"Models/escenaCompleta.obj");
    Model agua((char*)"Models/agua.obj");

    // Configuraci�n inicial del shader de iluminaci�n
    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

    // Configuraci�n del skybox (unidad de textura 0, ya que no compite con nada)
    skyboxShader.Use();
    glUniform1i(glGetUniformLocation(skyboxShader.Program, "skybox"), 0);

    glm::mat4 projection = glm::perspective(camera.GetZoom(),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

    // ============================================================
    // BUCLE PRINCIPAL
    // ============================================================
    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        // Color de fondo irrelevante: el skybox lo tapa todo
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ====================================================================
        // RENDERIZADO DEL SKYBOX / CAMPO DE ESTRELLAS
        // ====================================================================
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE); 

        skyboxShader.Use();
        glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix()));

        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(skyView));

        glm::mat4 modelSky = glm::mat4(1.0f);
        modelSky = glm::rotate(modelSky, (float)glfwGetTime() * 0.02f, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelSky));
        glUniform3f(glGetUniformLocation(skyboxShader.Program, "starColor"), 1.0f, 1.0f, 1.0f);
        skyStars.Draw(skyboxShader);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glm::mat4 view = camera.GetViewMatrix();

        // --------------------------------------------------------
        // ESCENA PRINCIPAL
        // --------------------------------------------------------
        lightingShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"),
            camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // --- LUZ DIRECCIONAL (sol fijo) ---
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"),
            dirLightDir.x, dirLightDir.y, dirLightDir.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.6f, 0.6f, 0.55f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 1.2f, 1.15f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.5f, 0.5f, 0.5f);

        // --- LUZ PUNTUAL: ELIMINADA (era la luna) ---
        // Si tu shader requiere que pointLights[0] tenga valores v�lidos, ponla a cero:
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.007f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.0002f);

        // --- SPOTLIGHT (desactivada) ---
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"),
            camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"),
            camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

        glUniform1i(glGetUniformLocation(lightingShader.Program, "activePointLights"), 12);

        // ====================================================================
        // 1. LUZ DIRECCIONAL (LUNA)
        // ====================================================================
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.05f, 0.05f, 0.12f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.15f, 0.20f, 0.35f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.1f, 0.1f, 0.2f);

        // ====================================================================
        // 2. CONFIGURAR LUZ
        // ====================================================================
        glm::vec3 farolPositions[12] = {
            glm::vec3(-10.0f, 2.0f, 3.0f), glm::vec3(10.0f, 2.0f, 3.0f),
            glm::vec3(-19.0f, 1.0f, 5.0f), glm::vec3(19.0f, 1.0f, 5.0f),
            glm::vec3(-30.0f, 2.0f, 3.0f), glm::vec3(30.0f, 2.0f, 3.0f),
            glm::vec3(-10.0f, 2.0f, 27.0f), glm::vec3(10.0f, 2.0f, 27.0f),
            glm::vec3(-30.0f, 2.0f, 27.0f), glm::vec3(30.0f, 2.0f, 27.0f),
            glm::vec3(0.0f, 0.0f, -43.0f), glm::vec3(0.0f, 0.0f, -43.0f)
        };

        for (int i = 0; i < 10; i++) {
            std::string number = std::to_string(i);

            glUniform3f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].position").c_str()), farolPositions[i].x, farolPositions[i].y, farolPositions[i].z);

            // Color de luz clida de farol
            glUniform3f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].ambient").c_str()), 0.1f, 0.08f, 0.05f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].diffuse").c_str()), 0.8f, 0.6f, 0.3f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].specular").c_str()), 0.8f, 0.6f, 0.3f);

            // Atenuacin
            glUniform1f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].linear").c_str()), 0.22f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].quadratic").c_str()), 0.019f);
        }

        for (int i = 10; i < 12; i++) {
            std::string number = std::to_string(i);

            glUniform3f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].position").c_str()), farolPositions[i].x, farolPositions[i].y, farolPositions[i].z);

            // Color de luz clida de farol
            glUniform3f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].ambient").c_str()), 0.1f, 0.08f, 0.05f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].diffuse").c_str()), 0.8f, 0.6f, 0.3f);
            glUniform3f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].specular").c_str()), 0.8f, 0.6f, 0.3f);

            // Atenuacin
            glUniform1f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].constant").c_str()), 1.0f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].linear").c_str()), 0.044f);
            glUniform1f(glGetUniformLocation(lightingShader.Program, ("pointLights[" + number + "].quadratic").c_str()), 0.0038f);
        }

        // Escena principal
        glUniform1i(glGetUniformLocation(lightingShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "useEmission"), 0); 

        glm::mat4 modelEscena = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelEscena));
        escena_principal.Draw(lightingShader);

        // Agua
        glUniform1i(glGetUniformLocation(lightingShader.Program, "isWater"), 1);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "uTime"), (float)glfwGetTime());
        glUniform1f(glGetUniformLocation(lightingShader.Program, "waveAmplitude"), 0.03f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "waveFrequency"), 1.5f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "waveSpeed"), 1.8f);
        glUniform2f(glGetUniformLocation(lightingShader.Program, "waterSpeed"), 0.02f, 0.01f);

        glm::mat4 modelPapel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPapel));
        papel.Draw(papelShader);

        // --------------------------------------------------------
        // SKYBOX (al final, con LEQUAL)
        // --------------------------------------------------------
        glDepthFunc(GL_LEQUAL);
        skyboxShader.Use();

        glm::mat4 viewSky = glm::mat4(glm::mat3(camera.GetViewMatrix()));

        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewSky));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
    }

    // Limpieza
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);

    glfwTerminate();
    return 0;
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])    camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])  camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])  camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)          keys[key] = true;
        else if (action == GLFW_RELEASE)   keys[key] = false;
    }
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = (GLfloat)xPos;
        lastY = (GLfloat)yPos;
        firstMouse = false;
    }

    GLfloat xOffset = (GLfloat)xPos - lastX;
    GLfloat yOffset = lastY - (GLfloat)yPos;

    lastX = (GLfloat)xPos;
    lastY = (GLfloat)yPos;

    if (person) {
        //giroMonito += (float)xoffset * 0.1;
        giroPerson -= (float)xOffset * mouseSensitivity;

        // El movimiento vertical inclina solamente la cmara
        cameraPitch += yOffset * mouseSensitivity * 3;

        // Limitar la inclinacin para evitar que la cmara d vueltas
        cameraPitch = glm::clamp(cameraPitch, -7.0f, 40.0f);
    }
    else {
        camera.ProcessMouseMovement(xOffset, yOffset);
    }
}