// Copyright (c) 2021 Guyutongxue
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <random>
#include <numbers>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "shader.h"
#include "model.hpp"
#include "light.hpp"

#include <imgui.h>
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// screen size
constexpr const auto SCR_WIDTH{800u};
constexpr const auto SCR_HEIGHT{600u};

// shadow map size
constexpr const auto RSM_WIDTH{1024u};
constexpr const auto RSM_HEIGHT{1024u};

// rsm sample settings
constexpr const auto MAX_SAMPLE_NUM{256u};
constexpr const auto MAX_SAMPLE_RADIUS{0.3f};

// camera settings
Camera camera({-40.0f, 15.0f, 15.0f});

auto deltaTime{0.0f};

// light settings
glm::vec3 lightPos(-20.0f, 20.0f, 20.0f);
glm::vec3 lightDiffuse(0.6f, 0.6f, 0.6f);
auto lightNearPlane{0.1f};
auto lightFarPlane{100.0f};

// how strong rsm is
auto indirectWeight{60.0f};

static void processInput(GLFWwindow* window);
static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

GLuint createRandomTexture(std::size_t size);

class Planes {
    GLuint groundVao, backwallVao, rightwallVao;
    GLuint groundVbo, backwallVbo, rightwallVbo;
    GLuint groundEbo, backwallEbo, rightwallEbo;

public:
    Planes() {
        // clang-format off
        std::array groundVertices{
            // position        //normal
             0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            -5.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            -5.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f,
             0.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f
        };
        std::array backwallVertices{
            // position        //normal
             0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
             0.0f, 5.0f, 0.0f,  0.0f, 0.0f, 1.0f,
            -5.0f, 5.0f, 0.0f,  0.0f, 0.0f, 1.0f,
            -5.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f
        };
        std::array rightwallVertices{
            // position        //normal
             0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 
             0.0f, 0.0f, 5.0f, -1.0f, 0.0f, 0.0f, 
             0.0f, 5.0f, 5.0f, -1.0f, 0.0f, 0.0f,
             0.0f, 5.0f, 0.0f, -1.0f, 0.0f, 0.0f
        };
        // clang-format on
        GLuint planeEbo[]{0, 1, 3, 1, 2, 3};
        // ground
        glGenVertexArrays(1, &groundVao);
        glGenBuffers(1, &groundVbo);
        glGenBuffers(1, &groundEbo);
        glBindVertexArray(groundVao);
        glBindBuffer(GL_ARRAY_BUFFER, groundVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeEbo), planeEbo, GL_STATIC_DRAW);
        // backwall
        glGenVertexArrays(1, &backwallVao);
        glGenBuffers(1, &backwallVbo);
        glGenBuffers(1, &backwallEbo);
        glBindVertexArray(backwallVao);
        glBindBuffer(GL_ARRAY_BUFFER, backwallVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(backwallVertices), backwallVertices.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backwallEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeEbo), planeEbo, GL_STATIC_DRAW);
        // rightwall
        glGenVertexArrays(1, &rightwallVao);
        glGenBuffers(1, &rightwallVbo);
        glGenBuffers(1, &rightwallEbo);
        glBindVertexArray(rightwallVao);
        glBindBuffer(GL_ARRAY_BUFFER, rightwallVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(rightwallVertices), rightwallVertices.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                              reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rightwallEbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeEbo), planeEbo, GL_STATIC_DRAW);
    }
    void draw(Shader& shader) {
        shader.use();

        shader.setUniform("model", glm::scale(glm::mat4(1.0f), {5, 5, 5}));
        // plane colors
        glBindVertexArray(groundVao);
        shader.setUniform("material.diffuse", 0.0f, 0.0f, 0.8f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(backwallVao);
        shader.setUniform("material.diffuse", 0.0f, 0.8f, 0.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(rightwallVao);
        shader.setUniform("material.diffuse", 0.8f, 0.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};

int main() {
    // initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window{
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Reflective Shadow Map", nullptr, nullptr)};
    if (!window) {
        std::cout << "Failed to Create glfw window\n";
        std::exit(EXIT_FAILURE);
    }
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwMakeContextCurrent(window);

    // initialize glad
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD\n";
        std::exit(EXIT_FAILURE);
    }

    // initialize imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // two shaders in rsm
    Shader mainShader("./main_shader.vert", "./main_shader.frag");
    Shader lightSpaceShader("./light_space_shader.vert", "./light_space_shader.frag");

    // things to render in each frame
    Planes planes;
    Model mainModel("./lumine.fbx");
    Light lightIndicator(lightPos, 5.0f);

    // create rsm framebuffers
    GLuint rsmFBO;
    GLuint depthMap, normalMap, worldPosMap, fluxMap;
    glGenFramebuffers(1, &rsmFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, rsmFBO);
    // create textures for rsm
    const glm::vec4 all1(1.0f, 1.0f, 1.0f, 1.0f);
    const glm::vec4 all0(0.0f, 0.0f, 0.0f, 0.0f);
    // depth
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, RSM_WIDTH, RSM_HEIGHT, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, reinterpret_cast<const float*>(&all0));
    // normal
    glGenTextures(1, &normalMap);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, RSM_WIDTH, RSM_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, reinterpret_cast<const float*>(&all0));
    // world coordinate
    glGenTextures(1, &worldPosMap);
    glBindTexture(GL_TEXTURE_2D, worldPosMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, RSM_WIDTH, RSM_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, reinterpret_cast<const float*>(&all1));
    // light flux
    glGenTextures(1, &fluxMap);
    glBindTexture(GL_TEXTURE_2D, fluxMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, RSM_WIDTH, RSM_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, reinterpret_cast<const float*>(&all0));

    // bind textures to framebuffers (set light_space_shader output)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, normalMap, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, worldPosMap, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fluxMap, 0);

    GLenum rsmDrawBuffers[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, rsmDrawBuffers);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // create random texture for random sampling
    GLuint randomMap = createRandomTexture(MAX_SAMPLE_NUM);

    // bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, worldPosMap);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, fluxMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, randomMap);

    // lightSpaceShader configuration
    const glm::mat4 lightProjection = glm::perspective(
        glm::radians(60.0f), 1.f * RSM_WIDTH / RSM_HEIGHT, lightNearPlane, lightFarPlane);
    glm::mat4 lightView;

    lightSpaceShader.use();
    lightSpaceShader.setUniform("light.diffuse", lightDiffuse);

    // mainShader configuration
    mainShader.use();
    mainShader.setUniform("material.ambient", 0.1f, 0.1f, 0.1f);
    mainShader.setUniform("material.specular", 0.1f, 0.1f, 0.1f);
    mainShader.setUniform("material.shininess", 8.0f);
    mainShader.setUniform("light.ambient", 0.2f, 0.2f, 0.2f);
    mainShader.setUniform("light.diffuse", lightDiffuse);
    mainShader.setUniform("light.specular", 1.0f, 1.0f, 1.0f);
    mainShader.setUniform("shadowNum", static_cast<int>(MAX_SAMPLE_NUM));
    mainShader.setUniform("shadowRadius", MAX_SAMPLE_RADIUS);
    mainShader.setUniform("shadowBias", 0.05f);

    mainShader.setUniform("depthMap", 0);
    mainShader.setUniform("normalMap", 1);
    mainShader.setUniform("worldPosMap", 2);
    mainShader.setUniform("fluxMap", 3);
    mainShader.setUniform("randomMap", 4);

    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Control");
        if (ImGui::SliderFloat3("Light Position", reinterpret_cast<float*>(&lightPos), -35.0f, 35.0f)) {
            lightIndicator.setPos(lightPos);
        }
        ImGui::SliderFloat("Reflectivity", reinterpret_cast<float*>(&indirectWeight), 10.0f,
                           100.0f);
        ImGui::End();
        ImGui::Render();

        // time
        static float lastFrame{0.0f};
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // rsm render
        glBindFramebuffer(GL_FRAMEBUFFER, rsmFBO);
        glViewport(0, 0, RSM_WIDTH, RSM_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightSpaceShader.use();
        lightSpaceShader.setUniform("lightSpaceMatrix", lightProjection * lightView);
        lightSpaceShader.setUniform("light.position", lightPos);
        glViewport(0, 0, RSM_WIDTH, RSM_HEIGHT);
        planes.draw(lightSpaceShader);
        mainModel.draw(lightSpaceShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mainShader.use();
        mainShader.setUniform("indirectWeight", indirectWeight);
        mainShader.setUniform("viewPos", camera.position);
        glm::mat4 cameraProjection =
            glm::perspective(glm::radians(camera.zoom), 1.f * SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 cameraView = camera.getViewMatrix();
        mainShader.setUniform("projection", cameraProjection);
        mainShader.setUniform("view", cameraView);
        mainShader.setUniform("light.position", lightPos);
        mainShader.setUniform("lightSpaceMatrix", lightProjection * lightView);
        planes.draw(mainShader);
        mainModel.draw(mainShader);

        lightIndicator.setMvp(cameraProjection * cameraView);
        lightIndicator.draw();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    std::exit(EXIT_SUCCESS);
}

static void processInput(GLFWwindow* window) {
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Forward, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Backward, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Left, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Right, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Up, cameraSpeed);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard(CamMove::Down, cameraSpeed);
}

static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX, lastY;
    static auto firstMouse{true};

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
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity{0.15f};
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.processMouseMovement(xoffset, yoffset);
}
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
        return;
    camera.processMouseScroll(yoffset);
}

GLuint createRandomTexture(std::size_t size) {
    static std::default_random_engine eng(std::random_device{}());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::vector<glm::vec3> randomData(size);
    for (auto& i : randomData) {
        float r1 = dist(eng);
        float r2 = dist(eng);
        i = {r1 * std::sin(2 * std::numbers::pi * r2), r1 * std::cos(2 * std::numbers::pi * r2),
             r1 * r1};
    }
    GLuint randomTexture;
    glGenTextures(1, &randomTexture);
    glBindTexture(GL_TEXTURE_2D, randomTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, size, 1, 0, GL_RGB, GL_FLOAT, randomData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return randomTexture;
}