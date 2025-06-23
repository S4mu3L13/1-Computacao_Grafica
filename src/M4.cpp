/* M4 - Cubo texturizado com iluminação Phong
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

// Transformações globais (controle teclado)
float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
float scaleFactor = 1.0f;
float posX = 0.0f, posY = 0.0f, posZ = 0.0f;

int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 800, "M4 - Cubo Texturizado com Phong", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Erro ao inicializar GLAD" << endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    GLuint shaderProgram = setupShader();
    GLuint VAO = setupGeometry();
    GLuint textureID = loadTexture("texturas/caixa.jpg");  // Coloque a textura nesta pasta

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    // Posição da luz e câmera
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
    glm::vec3 viewPos(0.0f, 0.0f, 3.0f);

    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    GLint ambientLoc = glGetUniformLocation(shaderProgram, "ambientColor");
    GLint diffuseLoc = glGetUniformLocation(shaderProgram, "diffuseColor");
    GLint specularLoc = glGetUniformLocation(shaderProgram, "specularColor");
    GLint shininessLoc = glGetUniformLocation(shaderProgram, "shininess");

    while (!glfwWindowShouldClose(window))
    {
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

        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));
        glUniform3f(ambientLoc, 0.2f, 0.2f, 0.2f);
        glUniform3f(diffuseLoc, 0.5f, 0.5f, 0.5f);
        glUniform3f(specularLoc, 1.0f, 1.0f, 1.0f);
        glUniform1f(shininessLoc, 32.0f);

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
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

GLuint setupShader()
{
    const char* vertexShaderSource = R\"(
    #version 450 core
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 color;
    layout(location = 2) in vec2 texCoord;
    layout(location = 3) in vec3 normal;

    out vec2 TexCoord;
    out vec3 Normal;
    out vec3 FragPos;

    uniform mat4 model;

    void main()
    {
        vec4 worldPos = model * vec4(position, 1.0);
        gl_Position = worldPos;
        FragPos = vec3(worldPos);
        Normal = mat3(transpose(inverse(model))) * normal;
        TexCoord = texCoord;
    }
    )\"";

    const char* fragmentShaderSource = R\"(
    #version 450 core

    in vec2 TexCoord;
    in vec3 Normal;
    in vec3 FragPos;

    uniform sampler2D texture1;
    uniform vec3 lightPos;
    uniform vec3 viewPos;

    uniform vec3 ambientColor;
    uniform vec3 diffuseColor;
    uniform vec3 specularColor;
    uniform float shininess;

    out vec4 color;

    void main()
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);

        float diff = max(dot(norm, lightDir), 0.0);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        vec3 ambient = ambientColor * vec3(texture(texture1, TexCoord));
        vec3 diffuse = diffuseColor * diff * vec3(texture(texture1, TexCoord));
        vec3 specular = specularColor * spec;

        vec3 result = ambient + diffuse + specular;
        color = vec4(result, 1.0);
    }
    )\"";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "Erro compilando Vertex Shader: " << infoLog << endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "Erro compilando Fragment Shader: " << infoLog << endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "Erro linkando shader program: " << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint setupGeometry()
{
    float vertices[] = {
        // pos(x,y,z)    color(r,g,b)    texCoord(u,v)   normal(x,y,z)

        // Back face
        -0.5f, -0.5f, -0.5f,   1,0,0,   0.0f, 0.0f,    0.0f, 0.0f,-1.0f,
         0.5f, -0.5f, -0.5f,   0,1,0,   1.0f, 0.0f,    0.0f, 0.0f,-1.0f,
         0.5f,  0.5f, -0.5f,   0,0,1,   1.0f, 1.0f,    0.0f, 0.0f,-1.0f,
         0.5f,  0.5f, -0.5f,   0,0,1,   1.0f, 1.0f,    0.0f, 0.0f,-1.0f,
        -0.5f,  0.5f, -0.5f,   1,1,0,   0.0f, 1.0f,    0.0f, 0.0f,-1.0f,
        -0.5f, -0.5f, -0.5f,   1,0,0,   0.0f, 0.0f,    0.0f, 0.0f,-1.0f,

        // Front face
        -0.5f, -0.5f,  0.5f,   1,0,1,   0.0f, 0.0f,    0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0,1,1,   1.0f, 0.0f,    0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   1,1,1,   1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,   1,1,1,   1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,   0,1,0,   0.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   1,0,1,   0.0f, 0.0f,    0.0f, 0.0f, 1.0f,

        // Left face
        -0.5f,  0.5f,  0.5f,   0,0,1,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   1,1,0,   1.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   1,0,1,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   1,0,1,   0.0f, 1.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0,1,1,   0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0,0,1,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,

        // Right face
         0.5f,  0.5f,  0.5f,   1,0,0,   1.0f, 0.0f,    1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0,1,0,   1.0f, 1.0f,    1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0,0,1,   0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0,0,1,   0.0f, 1.0f,    1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1,1,0,   0.0f, 0.0f,    1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1,0,0,   1.0f, 0.0f,    1.0f, 0.0f, 0.0f,

        // Bottom face
        -0.5f, -0.5f, -0.5f,   0,1,0,   0.0f, 1.0f,    0.0f,-1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   1,0,1,   1.0f, 1.0f,    0.0f,-1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0,0,1,   1.0f, 0.0f,    0.0f,-1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0,0,1,   1.0f, 0.0f,    0.0f,-1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   1,0,0,   0.0f, 0.0f,    0.0f,-1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,   0,1,0,   0.0f, 1.0f,    0.0f,-1.0f, 0.0f,

        // Top face
        -0.5f,  0.5f, -0.5f,   1,0,1,   0.0f, 1.0f,    0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0,1,0,   1.0f, 1.0f,    0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0,0,1,   1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0,0,1,   1.0f, 0.0f,    0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   1,1,0,   0.0f, 0.0f,    0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   1,0,1,   0.0f, 1.0f,    0.0f, 1.0f, 0.0f
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // normal
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    return VAO;
}

GLuint loadTexture(const char* path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Parâmetros da textura
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        cout << "Falha ao carregar textura: " << path << endl;
        stbi_image_free(data);
    }

    return textureID;
}
