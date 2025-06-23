// src/M5.cpp

#include <iostream>
#include <string>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

// --- Câmera em Primeira Pessoa ---
class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;

    float MovementSpeed;
    float MouseSensitivity;

    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : Front(glm::vec3(0, 0, -1)), MovementSpeed(3.0f), MouseSensitivity(0.1f)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(char direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == 'W') Position += Front * velocity;
        if (direction == 'S') Position -= Front * velocity;
        if (direction == 'A') Position -= Right * velocity;
        if (direction == 'D') Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }

        updateCameraVectors();
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

// --- Globals ---

const unsigned int WIDTH = 1000;
const unsigned int HEIGHT = 1000;

// Vertex Shader GLSL
const char* vertexShaderSource = R"(
#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragColor;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    fragColor = color;
}
)";

// Fragment Shader GLSL
const char* fragmentShaderSource = R"(
#version 450 core
in vec3 fragColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(fragColor, 1.0);
}
)";

// Prototipos
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* window, float deltaTime);
unsigned int compileShader(GLenum type, const char* source);
unsigned int createShaderProgram();

unsigned int setupCube();

// Variáveis para mouse e câmera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), 
              glm::vec3(0.0f, 1.0f, 0.0f), 
              -90.0f, 0.0f);

float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

int main()
{
    // Inicializa GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Camera FPS - Samuel", NULL, NULL);
    if (!window) {
        cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Callback para redimensionar janela
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Callback do mouse
    glfwSetCursorPosCallback(window, mouse_callback);

    // Captura o mouse na janela
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Inicializa GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // Compila e linka shaders
    unsigned int shaderProgram = createShaderProgram();

    // Configura geometria do cubo
    unsigned int cubeVAO = setupCube();

    // Para controlar o tempo entre frames (movimento suave)
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    glEnable(GL_DEPTH_TEST);

    // Loop principal
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        // Limpa tela e buffer de profundidade
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Matrizes
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // Envia para o shader
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Desenha cubo
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Troca buffers e processa eventos
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpeza
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard('W', deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard('S', deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard('A', deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard('D', deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // invertido y

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

unsigned int compileShader(GLenum type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        cout << "ERROR: Shader compilation failed\n" << infoLog << endl;
    }
    return shader;
}

unsigned int createShaderProgram()
{
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        cout << "ERROR: Shader linking failed\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

unsigned int setupCube()
{
    // Vertices do cubo: posição (x,y,z) + cor (r,g,b)
    float vertices[] = {
        // posição          // cor
        -0.5f,-0.5f,-0.5f,  1,0,0,
         0.5f,-0.5f,-0.5f,  0,1,0,
         0.5f, 0.5f,-0.5f,  0,0,1,
         0.5f, 0.5f,-0.5f,  0,0,1,
        -0.5f, 0.5f,-0.5f,  1,1,0,
        -0.5f,-0.5f,-0.5f,  1,0,0,

        -0.5f,-0.5f, 0.5f,  1,0,1,
         0.5f,-0.5f, 0.5f,  0,1,1,
         0.5f, 0.5f, 0.5f,  1,1,1,
         0.5f, 0.5f, 0.5f,  1,1,1,
        -0.5f, 0.5f, 0.5f,  0,1,0,
        -0.5f,-0.5f, 0.5f,  1,0,1,

        -0.5f, 0.5f, 0.5f,  0,0,1,
        -0.5f, 0.5f,-0.5f,  1,1,0,
        -0.5f,-0.5f,-0.5f,  1,0,1,
        -0.5f,-0.5f,-0.5f,  1,0,1,
        -0.5f,-0.5f, 0.5f,  0,1,1,
        -0.5f, 0.5f, 0.5f,  0,0,1,

         0.5f, 0.5f, 0.5f,  1,0,0,
         0.5f, 0.5f,-0.5f,  0,1,0,
         0.5f,-0.5f,-0.5f,  0,0,1,
         0.5f,-0.5f,-0.5f,  0,0,1,
         0.5f,-0.5f, 0.5f,  1,1,0,
         0.5f, 0.5f, 0.5f,  1,0,0,

        -0.5f,-0.5f,-0.5f,  0,1,0,
         0.5f,-0.5f,-0.5f,  1,0,1,
         0.5f,-0.5f, 0.5f,  0,0,1,
         0.5f,-0.5f, 0.5f,  0,0,1,
        -0.5f,-0.5f, 0.5f,  1,0,0,
        -0.5f,-0.5f,-0.5f,  0,1,0,

        -0.5f, 0.5f,-0.5f,  1,0,1,
         0.5f, 0.5f,-0.5f,  0,1,1,
         0.5f, 0.5f, 0.5f,  1,1,0,
         0.5f, 0.5f, 0.5f,  1,1,0,
        -0.5f, 0.5f, 0.5f,  0,1,0,
        -0.5f, 0.5f,-0.5f,  1,0,1
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return VAO;
}
