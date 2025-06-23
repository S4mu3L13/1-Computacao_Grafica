/* M3 - Visualizador com textura mapeada em cubo
 * Samuel Pasquali - Computação Gráfica
 */

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
GLuint setupShader();
GLuint setupGeometry();
GLuint loadTexture(const char* path);

float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
float scaleFactor = 1.0f;
float posX = 0.0f, posY = 0.0f, posZ = 0.0f;

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 800, "M3 - Cubo com Textura", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Erro ao inicializar GLAD" << endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    GLuint shaderProgram = setupShader();
    GLuint VAO = setupGeometry();
    GLuint textureID = loadTexture("texturas/caixa.jpg");

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(posX, posY, posZ));
        model = glm::rotate(model, angleX, glm::vec3(1, 0, 0));
        model = glm::rotate(model, angleY, glm::vec3(0, 1, 0));
        model = glm::rotate(model, angleZ, glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(scaleFactor));

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);
        if (key == GLFW_KEY_X) angleX += 0.1f;
        if (key == GLFW_KEY_Y) angleY += 0.1f;
        if (key == GLFW_KEY_Z) angleZ += 0.1f;

        if (key == GLFW_KEY_W) posZ -= 0.1f;
        if (key == GLFW_KEY_S) posZ += 0.1f;
        if (key == GLFW_KEY_A) posX -= 0.1f;
        if (key == GLFW_KEY_D) posX += 0.1f;
        if (key == GLFW_KEY_I) posY += 0.1f;
        if (key == GLFW_KEY_J) posY -= 0.1f;

        if (key == GLFW_KEY_LEFT_BRACKET) scaleFactor -= 0.1f;
        if (key == GLFW_KEY_RIGHT_BRACKET) scaleFactor += 0.1f;
    }
}

GLuint setupShader() {
    const char* vertexShaderSource = "#version 450 core\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec3 color;\n"
        "layout(location = 2) in vec2 texCoord;\n"
        "out vec2 TexCoord;\n"
        "uniform mat4 model;\n"
        "void main() {\n"
        "    gl_Position = model * vec4(position, 1.0);\n"
        "    TexCoord = texCoord;\n"
        "}\n";

    const char* fragmentShaderSource = "#version 450 core\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D texture1;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "    color = texture(texture1, TexCoord);\n"
        "}\n";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint setupGeometry() {
    float vertices[] = {
        // posições       // cores        // texCoords
        -0.5f, -0.5f, -0.5f, 1,0,0, 0,0,
         0.5f, -0.5f, -0.5f, 0,1,0, 1,0,
         0.5f,  0.5f, -0.5f, 0,0,1, 1,1,
         0.5f,  0.5f, -0.5f, 0,0,1, 1,1,
        -0.5f,  0.5f, -0.5f, 1,1,0, 0,1,
        -0.5f, -0.5f, -0.5f, 1,0,0, 0,0,

        -0.5f, -0.5f,  0.5f, 1,0,1, 0,0,
         0.5f, -0.5f,  0.5f, 0,1,1, 1,0,
         0.5f,  0.5f,  0.5f, 1,1,1, 1,1,
         0.5f,  0.5f,  0.5f, 1,1,1, 1,1,
        -0.5f,  0.5f,  0.5f, 0.5,0.5,0.5, 0,1,
        -0.5f, -0.5f,  0.5f, 1,0,1, 0,0,

        -0.5f,  0.5f,  0.5f, 1,1,0, 1,0,
        -0.5f,  0.5f, -0.5f, 1,0,1, 1,1,
        -0.5f, -0.5f, -0.5f, 0,1,1, 0,1,
        -0.5f, -0.5f, -0.5f, 0,1,1, 0,1,
        -0.5f, -0.5f,  0.5f, 1,1,1, 0,0,
        -0.5f,  0.5f,  0.5f, 1,1,0, 1,0,

         0.5f,  0.5f,  0.5f, 1,1,0, 1,0,
         0.5f,  0.5f, -0.5f, 1,0,0, 1,1,
         0.5f, -0.5f, -0.5f, 0,1,0, 0,1,
         0.5f, -0.5f, -0.5f, 0,1,0, 0,1,
         0.5f, -0.5f,  0.5f, 0,0,1, 0,0,
         0.5f,  0.5f,  0.5f, 1,1,0, 1,0,

        -0.5f, -0.5f, -0.5f, 1,1,1, 0,1,
         0.5f, -0.5f, -0.5f, 1,1,1, 1,1,
         0.5f, -0.5f,  0.5f, 1,1,1, 1,0,
         0.5f, -0.5f,  0.5f, 1,1,1, 1,0,
        -0.5f, -0.5f,  0.5f, 1,1,1, 0,0,
        -0.5f, -0.5f, -0.5f, 1,1,1, 0,1,

        -0.5f,  0.5f, -0.5f, 1,1,1, 0,1,
         0.5f,  0.5f, -0.5f, 1,1,1, 1,1,
         0.5f,  0.5f,  0.5f, 1,1,1, 1,0,
         0.5f,  0.5f,  0.5f, 1,1,1, 1,0,
        -0.5f,  0.5f,  0.5f, 1,1,1, 0,0,
        -0.5f,  0.5f, -0.5f, 1,1,1, 0,1,
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    return VAO;
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Erro ao carregar imagem: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}
