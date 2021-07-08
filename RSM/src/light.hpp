#include <glad/glad.h>
#include <glm/glm.hpp>

#include <array>

class Light {
private:
    static constexpr const char* const vertCode{R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 MVP;
void main() {
    gl_Position = MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)"};
    static constexpr const char* const fragCode{R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
})"};
    std::array<float, 3> vertices;
    float width;

    int shaderProgram;
    unsigned VBO, VAO;
    glm::mat4 mvp;
    static constexpr const glm::vec3 color{1.0f, 1.0f, 1.0f};

public:
    Light(glm::vec3 position, float width)
    : vertices{position.x, position.y, position.z} {
        glPointSize(width);

        // vertex shader
        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertCode, nullptr);
        glCompileShader(vertexShader);

        // fragment shader
        int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragCode, nullptr);
        glCompileShader(fragmentShader);

        // link shaders
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    int draw() {
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, &mvp[0][0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &color[0]);

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);
        return 0;
    }

    void setMvp(const glm::mat4 mvp) {
        this->mvp = mvp;
    }

    void setPos(const glm::vec3& position) {
        vertices = {position.x, position.y, position.z};
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    }

};