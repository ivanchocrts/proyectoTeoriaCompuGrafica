// Práctica Unificada: Iluminación Orbital y Animación de Agua
// Estudiante: Iván Daniel Cortés Alvarado

#include <iostream>
#include <string>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "SOIL2/SOIL2.h"
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// Dimensiones de ventana
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Prototipos
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void Animation();

// Cámara
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));
bool keys[1024];
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// Atributos de Luz y Cielo
glm::vec3 lightPos1(0.0f, 10.0f, 10.0f);
glm::vec3 lightPos2(0.0f, -10.0f, -10.0f);
glm::vec3 skyColor(0.1f, 0.1f, 0.12f); // Declaración única de skyColor

// DeltaTime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto - Escena Principal y Agua Animada. Ivan Daniel", nullptr, nullptr);

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

    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Opciones globales OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shaders
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

    // Modelos
    Model escena_principal((char*)"Models/escenaPrincipal.obj");
    Model agua((char*)"Models/agua.obj"); // Modelo de agua por separado
    Model sol((char*)"Models/modeloSol.obj");
    Model luna((char*)"Models/modeloLuna.obj");

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

    glm::mat4 projection = glm::perspective(camera.GetZoom(), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();
        Animation();

        // Limpiado de pantalla con el color dinámico del cielo
        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();

        // ====================================================================
        // DIBUJO DE ESCENA PRINCIPAL Y AGUA
        // ====================================================================
        lightingShader.Use();

        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // --- 1. LUZ DIRECCIONAL (SOL) ---
        glm::vec3 dirSol = glm::normalize(-lightPos1);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), dirSol.x, dirSol.y, dirSol.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.6f, 0.6f, 0.55f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 1.2f, 1.15f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.5f, 0.5f, 0.5f);

        // --- 2. LUZ PUNTUAL (LUNA) ---
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), lightPos2.x, lightPos2.y, lightPos2.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.2f, 0.2f, 0.35f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.6f, 0.7f, 1.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.007f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.0002f);

        // --- 3. SPOTLIGHT (DESACTIVADA) ---
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.09f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

        // --------------------------------------------------------------------
        // A1. DIBUJAR ESCENA PRINCIPAL (ESTÁTICA)
        // --------------------------------------------------------------------
        glUniform1i(glGetUniformLocation(lightingShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);

        glm::mat4 modelEscena(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelEscena));
        escena_principal.Draw(lightingShader);

        // --------------------------------------------------------------------
        // A2. DIBUJAR AGUA (ANIMADA)
        // --------------------------------------------------------------------
        glUniform1i(glGetUniformLocation(lightingShader.Program, "isWater"), 1);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "uTime"), (float)glfwGetTime());
        glUniform1f(glGetUniformLocation(lightingShader.Program, "waveAmplitude"), 0.03f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "waveFrequency"), 1.5f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "waveSpeed"), 1.8f);
        glUniform2f(glGetUniformLocation(lightingShader.Program, "waterSpeed"), 0.02f, 0.01f);

        glm::mat4 modelAgua(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelAgua));
        agua.Draw(lightingShader);

        // ====================================================================
        // DIBUJO DE LÁMPARAS (SOL Y LUNA)
        // ====================================================================
        lampShader.Use();
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // Sol
        glm::mat4 modeloSol(1.0f);
        modeloSol = glm::translate(modeloSol, lightPos1);
        modeloSol = glm::rotate(modeloSol, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        modeloSol = glm::scale(modeloSol, glm::vec3(1.5f, 1.5f, 1.5f));
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modeloSol));
        glUniform3f(glGetUniformLocation(lampShader.Program, "lightColor"), 1.0f, 0.7f, 0.0f);
        sol.Draw(lampShader);

        // Luna
        glm::mat4 modeloLuna(1.0f);
        modeloLuna = glm::translate(modeloLuna, lightPos2);
        modeloLuna = glm::rotate(modeloLuna, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        modeloLuna = glm::scale(modeloLuna, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(lampShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modeloLuna));
        glUniform3f(glGetUniformLocation(lampShader.Program, "lightColor"), 0.7f, 0.8f, 1.0f);
        luna.Draw(lampShader);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void Animation()
{
    // ------------------------------------------------------------------------
    // 1. Órbita Combinada (Circular/Translacional + Movimiento Vertical)
    // ------------------------------------------------------------------------
    float radiusX = 40.0f; // Radio de la órbita en el eje X
    float radiusZ = 30.0f; // Radio de la órbita en el eje Z (profundidad)
    float maxHeight = 35.0f; // Altura máxima en Y (Cénit del mediodía)

    float speed = 0.3f;    // Velocidad del ciclo día/noche
    float angle = (float)glfwGetTime() * speed;

    // El movimiento orbital traslada el objeto en X y Z mientras asciende/desciende en Y
    float solX = cos(angle) * radiusX;
    float solY = sin(angle) * maxHeight;
    float solZ = sin(angle) * radiusZ;

    // Posición del Sol
    lightPos1 = glm::vec3(solX, solY, solZ);

    // La Luna se coloca exactamente en el extremo opuesto de la órbita 3D
    lightPos2 = glm::vec3(-solX, -solY, -solZ);

    // ------------------------------------------------------------------------
    // 2. Transición dinámica del color del cielo (Día / Atardecer / Noche)
    // ------------------------------------------------------------------------
    // Normalizamos la altura vertical del Sol (solY) de [-maxHeight, maxHeight] a [0.0, 1.0]
    float factorDia = glm::clamp(solY / maxHeight, 0.0f, 1.0f);

    glm::vec3 colorNoche = glm::vec3(0.02f, 0.02f, 0.08f); // Azul noche oscuro
    glm::vec3 colorDia = glm::vec3(0.45f, 0.65f, 0.95f); // Azul cielo claro

    // Interpolación suave del color según el progreso diurno del Sol
    skyColor = glm::mix(colorNoche, colorDia, factorDia);
}

void DoMovement()
{
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) camera.ProcessKeyboard(RIGHT, deltaTime);

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

    camera.ProcessMouseMovement(xOffset, yOffset);
}