#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

// --- Câmera ---
class Camera {
public:
    glm::vec3 Position, Front, Up, Right, WorldUp;
    float Yaw, Pitch;
    float MovementSpeed = 3.0f;
    float MouseSensitivity = 0.1f;

    Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : Front(glm::vec3(0,0,-1))
    {
        Position = position; WorldUp = up;
        Yaw = yaw; Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() { return glm::lookAt(Position, Position + Front, Up); }

    void ProcessKeyboard(char dir, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (dir == 'W') Position += Front * velocity;
        if (dir == 'S') Position -= Front * velocity;
        if (dir == 'A') Position -= Right * velocity;
        if (dir == 'D') Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;

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

// --- Objeto com trajetória ---
class Objeto {
public:
    glm::vec3 position;
    glm::vec3 scale;
    vector<glm::vec3> pontosTrajetoria;
    int indiceAtual = 0;
    float velocidade = 2.0f; // unidades por segundo

    Objeto() {
        position = glm::vec3(0.0f);
        scale = glm::vec3(1.0f);
    }

    void atualizar(float deltaTime)
    {
        if (pontosTrajetoria.empty()) return;

        glm::vec3 destino = pontosTrajetoria[indiceAtual];
        glm::vec3 direcao = destino - position;
        float distancia = glm::length(direcao);

        if (distancia < 0.01f) {
            indiceAtual = (indiceAtual + 1) % pontosTrajetoria.size();
        }
        else {
            glm::vec3 movimento = glm::normalize(direcao) * velocidade * deltaTime;
            if (glm::length(movimento) > distancia)
                position = destino;
            else
                position += movimento;
        }
    }
};

// Globals
const unsigned int WIDTH = 1000, HEIGHT = 800;
Camera camera(glm::vec3(0, 0, 5), glm::vec3(0,1,0), -90.0f, 0.0f);
bool firstMouse = true;
float lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f;

float deltaTime = 0.0f, lastFrame = 0.0f;

Objeto cubo;

GLFWwindow* window;

// Shaders sources simples para cubo colorido
const char* vertexShaderSource = R"(
#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 450 core
in vec3 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

// Funções utilitárias
GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success; char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        cout << "Erro compilando shader: " << infoLog << endl;
    }
    return shader;
}

GLuint createShaderProgram()
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    int success; char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        cout << "Erro linkando programa: " << infoLog << endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

GLuint setupCube()
{
    // 8 vértices do cubo, 6 cores diferentes
    float vertices[] = {
        // pos           // cor
        -0.5f, -0.5f, -0.5f, 1,0,0,
         0.5f, -0.5f, -0.5f, 0,1,0,
         0.5f,  0.5f, -0.5f, 0,0,1,
        -0.5f,  0.5f, -0.5f, 1,1,0,
        -0.5f, -0.5f,  0.5f, 1,0,1,
         0.5f, -0.5f,  0.5f, 0,1,1,
         0.5f,  0.5f,  0.5f, 1,1,1,
        -0.5f,  0.5f,  0.5f, 0,0,0,
    };
    unsigned int indices[] = {
        0,1,2, 2,3,0, // atrás
        4,5,6, 6,7,4, // frente
        0,4,7, 7,3,0, // esquerda
        1,5,6, 6,2,1, // direita
        3,2,6, 6,7,3, // topo
        0,1,5, 5,4,0  // base
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return VAO;
}

// Callbacks e input
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        if(key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, true);

        // Adiciona ponto da trajetória com P
        if(key == GLFW_KEY_P)
        {
            cubo.pontosTrajetoria.push_back(camera.Position);
            cout << "Ponto adicionado: (" 
                 << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z << ")\n";
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    static bool firstMouse = true;
    static float lastX = WIDTH / 2.0f;
    static float lastY = HEIGHT / 2.0f;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // invert y
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard('W', deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard('S', deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard('A', deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard('D', deltaTime);
}

int main()
{
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(WIDTH, HEIGHT, "Cubo com Trajetoria - Samuel", NULL, NULL);
    if (window == NULL)
    {
        cout << "Falha ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Falha ao inicializar GLAD" << endl;
        return -1;
    }

    GLuint shaderProgram = createShaderProgram();
    GLuint cubeVAO = setupCube();

    glEnable(GL_DEPTH_TEST);

    cubo.position = glm::vec3(0.0f);
    cubo.scale = glm::vec3(1.0f);

    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        cubo.atualizar(deltaTime);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Matrizes
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubo.position);
        model = glm::scale(model, cubo.scale);

        // Passa uniform
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
