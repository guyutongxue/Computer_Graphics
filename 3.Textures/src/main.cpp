// Copyright (C) 2021 Guyutongxue
//
// This file is part of CGHomework/Textures.
//
// CGHomework/Textures is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CGHomework/Textures is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CGHomework/Textures.  If not, see <http://www.gnu.org/licenses/>.

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "camera.h"
#include "shader.h"
#include "light.hpp"

#include <imgui.h>
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void error_callback(int error, const char* description);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
void renderQuad();

// settings
constexpr const unsigned int SCR_WIDTH{800};
constexpr const unsigned int SCR_HEIGHT{800};

// camera
Camera camera({0.0f, 0.0f, 5.0f});
float lastX{SCR_WIDTH / 2.0f};
float lastY{SCR_HEIGHT / 2.0f};
bool firstMouse{true};

// timing
float deltaTime{0.0f};
float lastFrame{0.0f};

int main() {
    glfwSetErrorCallback(error_callback);

    // glfw: initialize and configure
    if (!glfwInit()) exit(EXIT_FAILURE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Textures", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(&glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ImGui Initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io{ImGui::GetIO()};
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // build and compile shaders
    Shader shader("vert.glsl", "frag.glsl");

    // load textures
    unsigned int diffuseMap = loadTexture("texture.bmp");
    unsigned int normalMap = loadTexture("texture_normal.bmp");

    // shader configuration
    shader.use();
    shader.setUniform("diffuseMap", 0);
    shader.setUniform("normalMap", 1);

    // lighting info
    glm::vec3 lightPos(1.5f, 1.5f, 1.5f);
    Light light(lightPos, 5.f);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Light control");
        if (ImGui::SliderFloat3("Position", reinterpret_cast<float*>(&lightPos), -7.0f, 7.0f)) {
            light.setPos(lightPos);
        }
        ImGui::End();
        ImGui::Render();

        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // configure view/projection matrices
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.zoom), static_cast<float>(SCR_WIDTH / SCR_HEIGHT), 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        shader.use();
        shader.setUniform("projection", projection);
        shader.setUniform("view", view);
        // render normal-mapped quad
        glm::mat4 model = glm::mat4(1.0f);
        // rotate the quad to show normal mapping from multiple directions
        model = glm::rotate(model, glm::radians<float>(glfwGetTime() * -10.0f),
                            glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
        shader.setUniform("model", model);
        shader.setUniform("viewPos", camera.position);
        shader.setUniform("lightPos", lightPos);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        renderQuad();

        light.setMvp(projection * view);
        light.draw();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

class Triangle {
private:
    glm::vec3 a, b, c;
    glm::vec2 uva, uvb, uvc;
    glm::vec3 nm;
    glm::vec3 tangent, bitangent;

public:
    Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec2 uva, glm::vec2 uvb, glm::vec2 uvc)
        : a{a}, b{b}, c{c}, uva{uva}, uvb{uvb}, uvc{uvc} {
        glm::vec3 edge1 = b - a;
        glm::vec3 edge2 = c - a;
        nm = glm::normalize(glm::cross(edge1, edge2));
        glm::vec2 deltaUv1 = uvb - uva;
        glm::vec2 deltaUv2 = uvc - uva;
        float f = 1.0f / (deltaUv1.x * deltaUv2.y - deltaUv2.x * deltaUv1.y);

        tangent.x = f * (deltaUv2.y * edge1.x - deltaUv1.y * edge2.x);
        tangent.y = f * (deltaUv2.y * edge1.y - deltaUv1.y * edge2.y);
        tangent.z = f * (deltaUv2.y * edge1.z - deltaUv1.y * edge2.z);

        bitangent.x = f * (-deltaUv2.x * edge1.x + deltaUv1.x * edge2.x);
        bitangent.y = f * (-deltaUv2.x * edge1.y + deltaUv1.x * edge2.y);
        bitangent.z = f * (-deltaUv2.x * edge1.z + deltaUv1.x * edge2.z);
    }

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 norm;
        glm::vec2 texCoord;
        glm::vec3 tangent, bitangent;
    };

    static_assert(sizeof(Vertex) == sizeof(float) * 14);

    operator std::array<Vertex, 3>() {
        return {{{a, nm, uva, tangent, bitangent},
                 {b, nm, uvb, tangent, bitangent},
                 {c, nm, uvc, tangent, bitangent}}};
    }
};

// renders a 1x1 quad in NDC with manually calculated tangent vectors
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
    if (quadVAO == 0) {
        // positions
        glm::vec3 pos1(-1.0f, 1.0f, 1.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 1.0f);
        glm::vec3 pos3(1.0f, -1.0f, 1.0f);
        glm::vec3 pos4(1.0f, 1.0f, 1.0f);
        glm::vec3 pos5(-1.0f, 1.0f, -1.0f);
        glm::vec3 pos6(-1.0f, -1.0f, -1.0f);
        glm::vec3 pos7(1.0f, -1.0f, -1.0f);
        glm::vec3 pos8(1.0f, 1.0f, -1.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);

        Triangle q1a{pos1, pos2, pos3, uv1, uv2, uv3};
        Triangle q1b{pos1, pos3, pos4, uv1, uv3, uv4};
        Triangle q2a{pos1, pos4, pos8, uv1, uv2, uv3};
        Triangle q2b{pos1, pos8, pos5, uv1, uv3, uv4};
        Triangle q3a{pos8, pos7, pos6, uv1, uv2, uv3};
        Triangle q3b{pos8, pos6, pos5, uv1, uv3, uv4};
        Triangle q4a{pos3, pos2, pos6, uv1, uv2, uv3};
        Triangle q4b{pos3, pos6, pos7, uv1, uv3, uv4};
        Triangle q5a{pos8, pos4, pos3, uv1, uv2, uv3};
        Triangle q5b{pos8, pos3, pos7, uv1, uv3, uv4};
        Triangle q6a{pos1, pos5, pos6, uv1, uv2, uv3};
        Triangle q6b{pos1, pos6, pos2, uv1, uv3, uv4};
        std::array<std::array<Triangle::Vertex, 3>, 12> quadVertices{q1a, q1b, q2a, q2b, q3a, q3b,
                                                                     q4a, q4b, q5a, q5b, q6a, q6b};

        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Triangle::Vertex),
                              (void*)(offsetof(Triangle::Vertex, pos)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Triangle::Vertex),
                              (void*)(offsetof(Triangle::Vertex, norm)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Triangle::Vertex),
                              (void*)(offsetof(Triangle::Vertex, texCoord)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Triangle::Vertex),
                              (void*)(offsetof(Triangle::Vertex, tangent)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Triangle::Vertex),
                              (void*)(offsetof(Triangle::Vertex, bitangent)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
    glBindVertexArray(0);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Forward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Backward, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Left, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Right, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Up, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Down, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
        firstMouse = true;
        return;
    }
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
        return;
    camera.processMouseScroll(yoffset);
}

static void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << "\n";
}

// utility function for loading a 2D texture from file
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        // use GL_CLAMP_TO_EDGE to prevent semi-transparent borders.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}