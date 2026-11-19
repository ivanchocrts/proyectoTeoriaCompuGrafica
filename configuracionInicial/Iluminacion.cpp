// Práctica Unificada: Iluminación Orbital y Animación de Agua
// Estudiante: Iván Daniel Cortés Alvarado
// commit de prueba
//agua modificada

#include <iostream>
#include <string>
#include <cmath>
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
#include "agua.h"		

// Dimensiones de ventana
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Prototipos
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void Animation();

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

// Atributos de Luz y Cielo
glm::vec3 lightPos1(0.0f, 10.0f, 10.0f);
glm::vec3 lightPos2(0.0f, -10.0f, -10.0f);
glm::vec3 skyColor(0.02f, 0.02f, 0.08f); 

// DeltaTime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// AGUA Objeto que controla el agua animada
AguaAnimada aguaAnimada;

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
    AguaAnimada::crearArchivosShader("Shader");						
    Shader aguaShader("Shader/agua.vs", "Shader/agua.fs");		

    // Modelos
    Model escena_principal((char*)"Models/principal/principal.obj");
    Model ofrenda((char*)"Models/ofrenda2/ofrenda2.obj");
    Model ofrenda2((char*)"Models/ofrenda2/ofrenda2.obj");
    Model flame((char*)"Models/flame/flame.obj");
    Model papel((char*)"Models/papel/papel.obj");

    // ANIMACIONES 
    Model agua((char*)"Models/agua.obj"); 
    Model sol((char*)"Models/modeloSol.obj");
    Model luna((char*)"Models/modeloLuna.obj");
    Model skyStars((char*)"Models/skybox/mc-stars.obj");
    Model Fantasmas((char*)"ModelosCarlos/3fantasmas/fantasmas_3.obj");
    ModelAnim modeloAnimado((char*)"Models/Mixamo/Greeting/Greeting.dae");
    modeloAnimado.initShaders(animShader.Program);

    ModelAnim animacion2((char*)"Models/Mixamo/Rumba/Rumba.dae");
    animacion2.initShaders(animShader.Program);

    //PERSONAJE
    Model CuerpoSkull((char*)"Models/skeleton/torso.obj");
    Model BrazoIzqSkull((char*)"Models/skeleton/brazoIzq.obj");
    Model BrazoDerSkull((char*)"Models/skeleton/brazoDer.obj");
    Model PiernaIzqSkull((char*)"Models/skeleton/pierIzq.obj");
    Model PiernaDerSkull((char*)"Models/skeleton/pierDer.obj");

    //ModelosExtra
    Model Traja_Gael((char*)"ModelosCarlos/traja1/trajinera1.obj");
    Model Traja_Kary((char*)"ModelosCarlos/traja1/trajinera2.obj");
    Model Traja_Carlos((char*)"ModelosCarlos/traja1/trajinera3.obj");
    Model Traja_Angel((char*)"ModelosCarlos/traja1/trajinera4.obj");

    //Inicializacin de KeyFrames
    for (int i = 0; i < MAX_FRAMES; i++)
    {
        KeyFrame[i].posX = 0;
        KeyFrame[i].posY = 0;
        KeyFrame[i].posZ = 0;
    }

    // KeyFrames para movimiento circular
    KeyFrame[0].posX = 0.0f;
    KeyFrame[0].posZ = 0.0f;
    KeyFrame[0].rotY = 0.0f;

    // Camina hacia adelante
    KeyFrame[1].posX = 0.0f;
    KeyFrame[1].posZ = 12.0f;
    KeyFrame[1].rotY = 0.0f;

    // SE DETIENE y Rota hacia la izquierda
    KeyFrame[2].posX = 0.0f;
    KeyFrame[2].posZ = 12.0f;
    KeyFrame[2].rotY = -90.0f;

    // Camina hacia la izquierda
    KeyFrame[3].posX = -19.0f;
    KeyFrame[3].posZ = 9.0f;
    KeyFrame[3].rotY = 180.0f;

    // SE DETIENE y Rota 180 grados hacia la derecha para regresar
    KeyFrame[4].posX = -20.0f;
    KeyFrame[4].posZ = 12.0f;
    KeyFrame[4].rotY = 90.0f;

    // Camina de regreso al centro
    KeyFrame[5].posX = 0.0f;
    KeyFrame[5].posZ = 12.0f;
    KeyFrame[5].rotY = 90.0f;

    // Sigue caminando hasta la derecha
    KeyFrame[6].posX = 19.0f;
    KeyFrame[6].posZ = 9.0f;
    KeyFrame[6].rotY = 180.0f;

    // Agregar giro
    KeyFrame[7].posX = 20.0f;
    KeyFrame[7].posZ = 12.0f;
    KeyFrame[7].rotY = -90.0f;

    // Camina al centro
    KeyFrame[8].posX = 0.0f;
    KeyFrame[8].posZ = 12.0f;
    KeyFrame[8].rotY = -90.0f;

    // Gira hacia donde antes estaba
    KeyFrame[9].posX = 0.0f;
    KeyFrame[9].posZ = 0.0f;
    KeyFrame[9].rotY = 0.0f;

    FrameIndex = MAX_FRAMES;

    {
        std::vector<glm::vec2> tris;
        for (int i = 0; i < NUM_TRIS_FOSO; i++)
        {
            tris.push_back(glm::vec2(FOSO_TRIS[i][0], FOSO_TRIS[i][1]));
            tris.push_back(glm::vec2(FOSO_TRIS[i][2], FOSO_TRIS[i][3]));
            tris.push_back(glm::vec2(FOSO_TRIS[i][4], FOSO_TRIS[i][5]));
        }
        aguaAnimada.initTriangulos(tris, NIVEL_AGUA, 45, 0.1f);
    }

    // [AGUA]
    aguaAnimada.amplitud = 0.05f; 
    aguaAnimada.frecuencia = 0.9f; 
    aguaAnimada.velocidad = 0.5f;
    aguaAnimada.transparencia = 0.88f;
    aguaAnimada.colorAgua = glm::vec3(0.04f, 0.17f, 0.26f);
    aguaAnimada.texturaDelModelo = false;


    // AGUA Parámetros de las olas. 
    //   amplitud   = qué tan altas son las olas
    //   frecuencia = qué tan juntas están (más chico = olas más anchas)
    //   velocidad  = qué tan rápido se mueven
    aguaAnimada.amplitud = 0.08f;
    aguaAnimada.frecuencia = 1.0f;
    aguaAnimada.velocidad = 1.8f;
    aguaAnimada.transparencia = 0.85f;
    aguaAnimada.texturaDelModelo = true;	
    // aguaAnimada.colorAgua = glm::vec3(0.05f, 0.35f, 0.55f);

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 500.0f);

    papelShader.Use();
    glUniform1i(glGetUniformLocation(papelShader.Program, "material.diffuse"), 0);

    flameShader.Use();
    glUniform1i(glGetUniformLocation(flameShader.Program, "material.diffuse"), 0);

    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();
        animate();
        cameraPerson();
        visuales();

        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
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

        // ====================================================================
        // DIBUJO DE ESCENA PRINCIPAL
        // ====================================================================
        lightingShader.Use();

        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

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

        // --------------------------------------------------------------------
        // DIBUJAR ESCENA PRINCIPAL (ESTTICA) Y OFRENDA
        // --------------------------------------------------------------------
        glUniform1i(glGetUniformLocation(lightingShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "useEmission"), 0); 

        glm::mat4 modelEscena = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelEscena));
        escena_principal.Draw(lightingShader);

        // (El agua ya NO se dibuja aquí: ahora se dibuja al final, con su propio shader)

        // ====================================================================
        // DIBUJAR FLAMAS DE LAS VELAS
        // ====================================================================
        flameShader.Use();

        glUniformMatrix4fv(glGetUniformLocation(flameShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(flameShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        float tiempoActual = (float)glfwGetTime();
        glUniform1f(glGetUniformLocation(flameShader.Program, "time"), tiempoActual);

        glm::mat4 modelFlame = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(flameShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelFlame));

        flame.Draw(flameShader);

        // ====================================================================
        // DIBUJAR PAPEL PICADO CON ONDEO
        // ====================================================================
        papelShader.Use();

        glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glUniform1f(glGetUniformLocation(papelShader.Program, "time"), (float)glfwGetTime());

        glUniform1f(glGetUniformLocation(papelShader.Program, "windStrength"), 0.15f); 
        glUniform1f(glGetUniformLocation(papelShader.Program, "waveSpeed"), 3.0f);     
        glUniform1f(glGetUniformLocation(papelShader.Program, "transparency"), 1.0f);

        glm::mat4 modelPapel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(papelShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelPapel));
        papel.Draw(papelShader);

        // ====================================================================
        // DIBUJO DE LMPARAS (SOL Y LUNA)
        // ====================================================================
        lampShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // -------------------------------------------------------------------------------------------------------------------------
        // Animacion Mixamo
        // -------------------------------------------------------------------------------------------------------------------------
        animShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(animShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(animShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(animShader.Program, "viewPos"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        glUniform3f(glGetUniformLocation(animShader.Program, "light.direction"), -0.2f, -1.0f, -0.3f);
        glUniform3f(glGetUniformLocation(animShader.Program, "light.ambient"), 0.2f, 0.2f, 0.2f);
        glUniform3f(glGetUniformLocation(animShader.Program, "light.diffuse"), 0.5f, 0.5f, 0.5f);
        glUniform3f(glGetUniformLocation(animShader.Program, "light.specular"), 1.0f, 1.0f, 1.0f);

        glUniform3f(glGetUniformLocation(animShader.Program, "material.specular"), 0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(animShader.Program, "material.shininess"), 32.0f);

        glm::mat4 modelMat = glm::mat4(1.0f);
        modelMat = glm::translate(modelMat, glm::vec3(5.0f, 0.0f, 23.0f));
        modelMat = glm::scale(modelMat, glm::vec3(3.0f));
        modelMat = glm::rotate(modelMat, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(animShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMat));

        modeloAnimado.Draw(animShader);

        modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, 23.0f));
        modelMat = glm::scale(modelMat, glm::vec3(3.0f));
        modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(animShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelMat));

        animacion2.Draw(animShader);


        // -------------------------------------------------------------------------------------------------------------------------
        // Fin Animacion Mixamo
        // -------------------------------------------------------------------------------------------------------------------------

        // ====================================================================
        // AGUA ANIMADA. 
        // ====================================================================
        aguaAnimada.luzDir = glm::vec3(-0.2f, -1.0f, -0.3f);
        aguaAnimada.luzColor = glm::vec3(0.1f, 0.1f, 0.2f);

        aguaAnimada.draw(aguaShader, glm::mat4(1.0f), view, projection, camera.GetPosition(), (float)glfwGetTime());

        // ====================================================================
        // ANIMACION FANTASMAS
        // ====================================================================

        glm::mat4 fantasmas_3(1.0f);
        lightingShader.Use();

        glUniform1i(glGetUniformLocation(lightingShader.Program, "useEmission"), 1);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "emissionColor"), 0.1f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "emissionStrength"), 0.30f);

        float flotacion = sin(glfwGetTime() * 2.0f) * 0.3f;

        fantasmas_3 = glm::translate(fantasmas_3, glm::vec3(posX, posY + flotacion, posZ));
        fantasmas_3 = glm::rotate(fantasmas_3, glm::radians(giroFantasma), glm::vec3(0.0f, 1.0f, 0.0f));
        fantasmas_3 = glm::scale(fantasmas_3, glm::vec3(0.044f));

        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(fantasmas_3));
        Fantasmas.Draw(lightingShader);

        glUniform1i(glGetUniformLocation(lightingShader.Program, "useEmission"), 0);

        // ====================================================================
        // DIBUJO DE TRAJINERAS (ANIMACIN DE FLOTACIN Y BALANCEO)
        // ====================================================================
        float tiempoTraja = (float)glfwGetTime();

        // TRAJA 1: GAEL 
        float oscY1 = sin(tiempoTraja * 1.5f) * 0.1f;
        float balZ1 = cos(tiempoTraja * 1.0f) * 3.0f;

        glm::mat4 modelTraja1 = glm::mat4(1.0f);
        modelTraja1 = glm::translate(modelTraja1, glm::vec3(trajaX, oscY1, trajaZ));
        modelTraja1 = glm::rotate(modelTraja1, glm::radians(trajaRotY), glm::vec3(0.0f, 1.0f, 0.0f));
        modelTraja1 = glm::rotate(modelTraja1, glm::radians(balZ1), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTraja1 = glm::scale(modelTraja1, glm::vec3(0.11f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelTraja1));
        Traja_Gael.Draw(lightingShader);


        // TRAJA 2: KARY 
        float oscY2 = sin((tiempoTraja + 1.0f) * 1.5f) * 0.1f;
        float balZ2 = cos((tiempoTraja + 1.0f) * 1.0f) * 3.0f;

        glm::mat4 modelTraja3 = glm::mat4(1.0f);
        modelTraja3 = glm::translate(modelTraja3, glm::vec3(-30.0f, oscY2, -59.0f));
        modelTraja3 = glm::rotate(modelTraja3, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelTraja3 = glm::rotate(modelTraja3, glm::radians(balZ2), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTraja3 = glm::scale(modelTraja3, glm::vec3(0.11f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelTraja3));
        Traja_Carlos.Draw(lightingShader);


        // TRAJA 3: CARLOS 
        float oscY3 = sin((tiempoTraja + 2.0f) * 1.5f) * 0.1f;
        float balZ3 = cos((tiempoTraja + 2.0f) * 1.0f) * 3.0f;

        glm::mat4 modelTraja2 = glm::mat4(1.0f);
        modelTraja2 = glm::translate(modelTraja2, glm::vec3(8.0f, oscY3, -20.0f));
        modelTraja2 = glm::rotate(modelTraja2, glm::radians(-20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelTraja2 = glm::rotate(modelTraja2, glm::radians(balZ3), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTraja2 = glm::scale(modelTraja2, glm::vec3(0.11f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelTraja2));
        Traja_Kary.Draw(lightingShader);


        // TRAJA 4: ANGEL 
        float oscY4 = sin((tiempoTraja + 3.0f) * 1.5f) * 0.1f;
        float balZ4 = cos((tiempoTraja + 3.0f) * 1.0f) * 3.0f;

        glm::mat4 modelTraja4 = glm::mat4(1.0f);
        modelTraja4 = glm::translate(modelTraja4, glm::vec3(30.0f, oscY4, -59.0f));
        modelTraja4 = glm::rotate(modelTraja4, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelTraja4 = glm::rotate(modelTraja4, glm::radians(balZ4), glm::vec3(0.0f, 0.0f, 1.0f));
        modelTraja4 = glm::scale(modelTraja4, glm::vec3(0.11f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelTraja4));
        Traja_Angel.Draw(lightingShader);

        if (person) {
            lightingShader.Use();

            float balanceoBrazos = 0.0f;
            float balanceoPiernas = 0.0f;
            float reboteTorso = 0.0f;

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            {
                float velocidadAnim = 10.0f;
                float maxGiroBrazos = 20.0f;
                float maxGiroPiernas = 20.0f;

                balanceoBrazos = sin(glfwGetTime() * velocidadAnim) * maxGiroBrazos;
                balanceoPiernas = sin(glfwGetTime() * velocidadAnim) * maxGiroPiernas;
                reboteTorso = abs(sin(glfwGetTime() * velocidadAnim)) * 0.1f;
            }

            glm::mat4 refCuerpo = glm::mat4(1.0f);

            refCuerpo = glm::translate(refCuerpo, glm::vec3(personX, personY + reboteTorso - 1.0f, personZ));
            refCuerpo = glm::rotate(refCuerpo, glm::radians(giroPerson), glm::vec3(0.0f, 1.0f, 0.0f));
            refCuerpo = glm::scale(refCuerpo, glm::vec3(2.0f));

            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(refCuerpo));
            CuerpoSkull.Draw(lightingShader);

            // ====================================================================
            // 2. BRAZO IZQUIERDO
            // ====================================================================
            glm::mat4 modelOp = glm::mat4(1.0f);
            modelOp = glm::translate(refCuerpo, glm::vec3(0.0f, 0.009f, 0.0f));
            modelOp = glm::rotate(modelOp, glm::radians(-balanceoBrazos), glm::vec3(1.0f, 0.0f, 0.0f));

            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelOp));
            BrazoIzqSkull.Draw(lightingShader);

            // ====================================================================
            // 3. BRAZO DERECHO
            // ====================================================================
            modelOp = glm::mat4(1.0f);
            modelOp = glm::translate(refCuerpo, glm::vec3(-0.0f, 0.025f, 0.0f));
            modelOp = glm::rotate(modelOp, glm::radians(balanceoBrazos), glm::vec3(1.0f, 0.0f, 0.0f));

            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelOp));
            BrazoDerSkull.Draw(lightingShader);

            // ====================================================================
            // 4. PIERNA IZQUIERDA
            // ====================================================================
            modelOp = glm::mat4(1.0f);
            modelOp = glm::translate(refCuerpo, glm::vec3(0.0f, 0.008f, 0.0f));
            modelOp = glm::rotate(modelOp, glm::radians(balanceoPiernas), glm::vec3(1.0f, 0.0f, 0.0f));

            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelOp));
            PiernaIzqSkull.Draw(lightingShader);

            // ====================================================================
            // 5. PIERNA DERECHA
            // ====================================================================
            modelOp = glm::mat4(1.0f);
            modelOp = glm::translate(refCuerpo, glm::vec3(-0.0f, 0.024f, 0.0f));
            modelOp = glm::rotate(modelOp, glm::radians(-balanceoPiernas), glm::vec3(1.0f, 0.0f, 0.0f));

            glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelOp));
            PiernaDerSkull.Draw(lightingShader);
        }

        // ====================================================================
        // DETECCION DE PROXIMIDAD Y TEXTO EN LA PARTE BAJA DE LA CAMARA
        // ====================================================================
        glm::vec3 posJugador = person ? glm::vec3(personX, personY, personZ) : camera.GetPosition();
        glm::vec2 posJugXZ(posJugador.x, posJugador.z);

        float distCatrina = glm::distance(posJugXZ, glm::vec2(-5.0f, 23.0f));

        float distCatrin = glm::distance(posJugXZ, glm::vec2(5.0f, 23.0f));

        float distOfrenda1 = glm::distance(posJugXZ, glm::vec2(19.0f, 5.0f));
        float distOfrenda2 = glm::distance(posJugXZ, glm::vec2(-19.0f, 5.0f));

        float distTraja1 = glm::distance(posJugXZ, glm::vec2(trajaX, trajaZ));
        float distTraja2 = glm::distance(posJugXZ, glm::vec2(8.0f, -20.0f));
        float distTraja3 = glm::distance(posJugXZ, glm::vec2(-30.0f, -59.0f));
        float distTraja4 = glm::distance(posJugXZ, glm::vec2(30.0f, -59.0f));

        float distCastillo = glm::distance(posJugXZ, glm::vec2(0.0f, -43.0f));

        // Radios de activación
        float radioPersonajes = 4.0f;
        float radioOfrenda = 7.0f;
        float radioTrajinera = 9.0f;
        float radioCastillo = 15.0f;

        if (distCatrina < radioPersonajes) {
            hudTexto.drawTextBottom(//________________________________________
                "LA CATRINA - Icono tradicional del Dia de Muertos creado por",
                "Jose Guadalupe Posada, representa la elegancia de la muerte.",
                SCREEN_WIDTH, SCREEN_HEIGHT, 1.9f
            );
        }
        else if (distCatrin < radioPersonajes) {
            hudTexto.drawTextBottom(//________________________________________
                "EL CATRIN - Figura clasica del Dia de Muertos que viste gala",
                "con traje formal y sombrero de copa para celebrar la muerte.",
                SCREEN_WIDTH, SCREEN_HEIGHT, 1.9f
            );
        }
        else if (distOfrenda1 < radioOfrenda || distOfrenda2 < radioOfrenda) {
            hudTexto.drawTextBottom(//________________________________________
                "LA OFRENDA - Altar sagrado con flores de cempasuchil y velas",
                "que iluminan el camino de las almas de regreso a su familia.",
                SCREEN_WIDTH, SCREEN_HEIGHT, 1.9f
            );
        }
        else if (distTraja1 < radioTrajinera || distTraja2 < radioTrajinera ||
            distTraja3 < radioTrajinera || distTraja4 < radioTrajinera) {
            hudTexto.drawTextBottom(//________________________________________
                "LA TRAJINERA - Embarcacion tipica mexicana adornada con",
                "colores vivos que navega con musica sobre el agua del lago",
                SCREEN_WIDTH, SCREEN_HEIGHT, 1.9f
            );
        }
        else if (distCastillo < radioCastillo) {
            hudTexto.drawTextBottom(//________________________________________
                "CASTILLO DE CHAUTLA - Emblematica construccion de estilo muy",
                "ingles del siglo XIX rodeada por un hermoso lago en Puebla. ",
                SCREEN_WIDTH, SCREEN_HEIGHT, 1.9f
            );
        }

        // ====================================================================
        // [AGUA] AGUA ANIMADA (se dibuja al final porque es transparente)
        // ====================================================================
        // La luz del agua sigue al sol: de día brilla, de noche se oscurece
        aguaAnimada.luzDir = dirSol;
        float intensidadSol = glm::clamp(lightPos1.y / 35.0f, 0.15f, 1.0f);
        aguaAnimada.luzColor = glm::vec3(1.0f, 0.97f, 0.9f) * intensidadSol;

        glm::mat4 modelAgua(1.0f);	// si necesitas mover o escalar el agua, hazlo aquí
        aguaAnimada.drawModelo(aguaShader, agua, modelAgua, view, projection,
            camera.GetPosition(), (float)glfwGetTime());

        glfwSwapBuffers(window);
    }

    // Audio
    ma_sound_uninit(&musicaFondo);
    ma_engine_uninit(&audioEngine);

    glfwTerminate();
    return 0;
}

void Animation()
{
    lightPos1 = glm::vec3(0.0f, 10.0f, 10.0f);
    lightPos2 = glm::vec3(0.0f, -10.0f, -10.0f);
    skyColor = glm::vec3(0.02f, 0.02f, 0.08f);
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] && person == false || keys[GLFW_KEY_UP] && person == false)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] && person == false || keys[GLFW_KEY_DOWN] && person == false)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] && person == false || keys[GLFW_KEY_LEFT] && person == false)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] && person == false || keys[GLFW_KEY_RIGHT] && person == false)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (keys[GLFW_KEY_I]) lightPos1.y += 0.05f;
    if (keys[GLFW_KEY_K]) lightPos1.y -= 0.05f;
    if (keys[GLFW_KEY_J]) lightPos1.x -= 0.05f;
    if (keys[GLFW_KEY_L]) lightPos1.x += 0.05f;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
        {
            keys[key] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            keys[key] = false;
        }
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        if (play == false && (FrameIndex > 1))
        {
            std::cout << "Play animation" << std::endl;
            resetElements();			
            interpolation();

            play = true;
            playIndex = 0;
            i_curr_steps = 0;
        }
        else
        {
            std::cout << "Not enough Key Frames" << std::endl;
            play = false;
        }
    }

    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        person = true;

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
        person = false;


    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        person = false;
        verEscen = true;
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        person = false;
        verAnim = true;
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        person = false;
        verModel = true;
    }

    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        animTraja = !animTraja;
    }

    float velocidad = 8.0f * deltaTime;

    float angulo = glm::radians(giroPerson);

    glm::vec3 direccion(
        sin(angulo),
        0.0f,
        cos(angulo)
    );

    // Movimiento hacia adelante
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && person == true)
    {
        personX += direccion.x * velocidad;
        personZ += direccion.z * velocidad;
        mov = true;
    }

    // Movimiento hacia atrs
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && person == true)
    {
        personX -= direccion.x * velocidad;
        personZ -= direccion.z * velocidad;
        mov = true;
    }

    // Movimiento hacia la derecha
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && person == true)
    {
        personX -= direccion.z * velocidad;
        personZ += direccion.x * velocidad;
        mov = true;
    }

    // Movimiento hacia la izquierda
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && person == true)
    {
        personX += direccion.z * velocidad;
        personZ -= direccion.x * velocidad;
        mov = true;
    }

    // To play/pause music
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        musicaActiva = !musicaActiva;
        if (musicaActiva) ma_sound_start(&musicaFondo);
        else              ma_sound_stop(&musicaFondo);
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