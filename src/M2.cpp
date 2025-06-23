// M2.cpp - Visualizador de cubo 3D com translação, rotação, escala e múltiplas instâncias
// Adaptado por ChatGPT para Samuel com base no template da professora Rossana

#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

const GLuint WIDTH = 800, HEIGHT = 600;

// Estados globais
float scale = 1.0f;
glm::vec3 movement(0.0f);
bool rotateX = false, rotateY = false, rotateZ = false;

// Instâncias de cubos
vector<glm::vec3> cubePositions = { glm::vec3(0.0f) };

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint setupShader();
GLuint setupGeometry();

const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"    gl_Position = projection * view * model * vec4(position, 1.0);\n"
"    finalColor = vec4(color, 1.0);\n"
"}\0";

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    color = finalColor;\n"
"}\n\0";

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Cubo 3D - M2", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Erro ao carregar GLAD" << endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    GLuint shaderProgram = setupShader();
    GLuint VAO = setupGeometry();

    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        float time = glfwGetTime();

        for (glm::vec3& pos : cubePositions) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos + movement);
            if (rotateX) model = glm::rotate(model, time, glm::vec3(1, 0, 0));
            if (rotateY) model = glm::rotate(model, time, glm::vec3(0, 1, 0));
            if (rotateZ) model = glm::rotate(model, time, glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(scale));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GL_TRUE);
        else if (key == GLFW_KEY_X) rotateX = true, rotateY = rotateZ = false;
        else if (key == GLFW_KEY_Y) rotateY = true, rotateX = rotateZ = false;
        else if (key == GLFW_KEY_Z) rotateZ = true, rotateX = rotateY = false;
        else if (key == GLFW_KEY_W) movement.z -= 0.1f;
        else if (key == GLFW_KEY_S) movement.z += 0.1f;
        else if (key == GLFW_KEY_A) movement.x -= 0.1f;
        else if (key == GLFW_KEY_D) movement.x += 0.1f;
        else if (key == GLFW_KEY_I) movement.y += 0.1f;
        else if (key == GLFW_KEY_J) movement.y -= 0.1f;
        else if (key == GLFW_KEY_LEFT_BRACKET) scale *= 0.95f;
        else if (key == GLFW_KEY_RIGHT_BRACKET) scale *= 1.05f;
        else if (key == GLFW_KEY_C) cubePositions.push_back(glm::vec3(0));
    }
}

GLuint setupShader() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

GLuint setupGeometry() {
    float vertices[] = {
        // posição           // cor por face
        // frente (vermelha)
        -0.5, -0.5,  0.5, 1,0,0,  0.5, -0.5,  0.5, 1,0,0,  0.5,  0.5,  0.5, 1,0,0,
         0.5,  0.5,  0.5, 1,0,0, -0.5,  0.5,  0.5, 1,0,0, -0.5, -0.5,  0.5, 1,0,0,
        // trás (verde)
        -0.5, -0.5, -0.5, 0,1,0,  0.5, -0.5, -0.5, 0,1,0,  0.5,  0.5, -0.5, 0,1,0,
         0.5,  0.5, -0.5, 0,1,0, -0.5,  0.5, -0.5, 0,1,0, -0.5, -0.5, -0.5, 0,1,0,
        // esquerda (azul)
        -0.5, -0.5, -0.5, 0,0,1, -0.5, -0.5,  0.5, 0,0,1, -0.5,  0.5,  0.5, 0,0,1,
        -0.5,  0.5,  0.5, 0,0,1, -0.5,  0.5, -0.5, 0,0,1, -0.5, -0.5, -0.5, 0,0,1,
        // direita (amarelo)
         0.5, -0.5, -0.5, 1,1,0,  0.5, -0.5,  0.5, 1,1,0,  0.5,  0.5,  0.5, 1,1,0,
         0.5,  0.5,  0.5, 1,1,0,  0.5,  0.5, -0.5, 1,1,0,  0.5, -0.5, -0.5, 1,1,0,
        // cima (magenta)
        -0.5,  0.5, -0.5, 1,0,1,  0.5,  0.5, -0.5, 1,0,1,  0.5,  0.5,  0.5, 1,0,1,
         0.5,  0.5,  0.5, 1,0,1, -0.5,  0.5,  0.5, 1,0,1, -0.5,  0.5, -0.5, 1,0,1,
        // baixo (ciano)
        -0.5, -0.5, -0.5, 0,1,1,  0.5, -0.5, -0.5, 0,1,1,  0.5, -0.5,  0.5, 0,1,1,
         0.5, -0.5,  0.5, 0,1,1, -0.5, -0.5,  0.5, 0,1,1, -0.5, -0.5, -0.5, 0,1,1,
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return VAO;
}
