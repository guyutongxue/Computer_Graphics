// Copyright (C) 2021 Guyutongxue
//
// This file is part of CGHomework/Hand.
//
// CGHomework/Hand is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CGHomework/Hand is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CGHomework/Hand.  If not, see <http://www.gnu.org/licenses/>.

#define _CRT_SECURE_NO_WARNINGS

//#define DIFFUSE_TEXTURE_MAPPING

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <glm\gtc\matrix_transform.hpp>
#include <iostream>
#include <numbers>

#include "skeletal_mesh.h"

namespace SkeletalAnimation {
const char* vertex_shader =
    "#version 330 core\n"
    "const int MAX_BONES = 100;\n"
    "uniform mat4 u_bone_transf[MAX_BONES];\n"
    "uniform mat4 u_mvp;\n"
    "layout(location = 0) in vec3 in_position;\n"
    "layout(location = 1) in vec2 in_texcoord;\n"
    "layout(location = 2) in vec3 in_normal;\n"
    "layout(location = 3) in ivec4 in_bone_index;\n"
    "layout(location = 4) in vec4 in_bone_weight;\n"
    "out vec2 pass_texcoord;\n"
    "void main() {\n"
    "    float adjust_factor = 0.0;\n"
    "    for (int i = 0; i < 4; i++) adjust_factor += in_bone_weight[i] * 0.25;\n"
    "    mat4 bone_transform = mat4(1.0);\n"
    "    if (adjust_factor > 1e-3) {\n"
    "        bone_transform -= bone_transform;\n"
    "        for (int i = 0; i < 4; i++)\n"
    "            bone_transform += u_bone_transf[in_bone_index[i]] * in_bone_weight[i] / "
    "adjust_factor;\n"
    "	 }\n"
    "    gl_Position = u_mvp * bone_transform * vec4(in_position, 1.0);\n"
    "    pass_texcoord = in_texcoord;\n"
    "}\n";

const char* fragment_shader =
    "#version 330 core\n"
    "uniform sampler2D u_diffuse;\n"
    "in vec2 pass_texcoord;\n"
    "out vec4 out_color;\n"
    "void main() {\n"
#ifdef DIFFUSE_TEXTURE_MAPPING
    "    out_color = vec4(texture(u_diffuse, pass_texcoord).xyz, 1.0);\n"
#else
    "    out_color = vec4(pass_texcoord, 0.0, 1.0);\n"
#endif
    "}\n";
}  // namespace SkeletalAnimation

bool firstMouse{true};
float yaw{90.0f};
float pitch{0.0f};
float lastX{800.0f / 2.0};
float lastY{600.0 / 2.0};
float fov{45.0f};

glm::fvec3 cameraPos(0.f, 7.f, -30.f);
glm::fvec3 cameraFront(0.f, 0.f, 1.f);
glm::fvec3 cameraUp(0.f, 1.f, 0.f);

glm::fmat4 modelRotation(1.0f);

float deltaTime{0.0f};
float lastFrame{0.0f};

bool enableMetacarpalsRotation{true};

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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

    float sensitivity = 0.1f;  // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
        return;
    fov -= (float)yoffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

static void processKeyboardInput(GLFWwindow* window) {
    float speed{8.f * deltaTime};
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) cameraPos += speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) cameraPos -= speed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) cameraPos += speed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) cameraPos -= speed * cameraUp;

    auto cameraLeft{glm::normalize(glm::cross(cameraFront, cameraUp))};
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) cameraPos -= speed * cameraLeft;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) cameraPos += speed * cameraLeft;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
            modelRotation =
                glm::rotate(glm::fmat4(1.0f), float(std::numbers::pi), glm::fvec3(0.f, 0.f, 1.f));
        } else {
            modelRotation = glm::rotate(glm::fmat4(1.0f), float(-std::numbers::pi / 2),
                                        glm::fvec3(0.f, 0.f, 1.f));
        }
    } else if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
        modelRotation =
            glm::rotate(glm::fmat4(1.0f), float(std::numbers::pi / 2), glm::fvec3(0.f, 0.f, 1.f));
    } else {
        modelRotation = glm::fmat4(1.0f);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

SkeletonModifier modifier;

enum HandBone {
    Metacarpals,
    ThumbProximalPhalange,
    ThumbIntermediatePhalange,
    ThumbDistalPhalange,
    ThumbFingertip,
    IndexProximalPhalange,
    IndexIntermediatePhalange,
    IndexDistalPhalange,
    IndexFingertip,
    MiddleProximalPhalange,
    MiddleIntermediatePhalange,
    MiddleDistalPhalange,
    MiddleFingertip,
    RingProximalPhalange,
    RingIntermediatePhalange,
    RingDistalPhalange,
    RingFingertip,
    PinkyProximalPhalange,
    PinkyIntermediatePhalange,
    PinkyDistalPhalange,
    PinkyFingertip,
    Last  // psuedo end item
};

struct Gesture {
    const char* name;
    float startTime;
    std::unordered_map<HandBone, float> angles;
};

std::unordered_map<HandBone, float> lastPosition;
std::optional<Gesture> currGesture{std::nullopt};

const float gestureDuration{0.5f};

static glm::fmat4& getModifier(HandBone bone) {
    const char* key{""};
    switch (bone) {
        case Metacarpals: key = "metacarpals"; break;
        case ThumbProximalPhalange: key = "thumb_proximal_phalange"; break;
        case ThumbIntermediatePhalange: key = "thumb_intermediate_phalange"; break;
        case ThumbDistalPhalange: key = "thumb_distal_phalange"; break;
        case ThumbFingertip: key = "thumb_fingertip"; break;
        case IndexProximalPhalange: key = "index_proximal_phalange"; break;
        case IndexIntermediatePhalange: key = "index_intermediate_phalange"; break;
        case IndexDistalPhalange: key = "index_distal_phalange"; break;
        case IndexFingertip: key = "index_fingertip"; break;
        case MiddleProximalPhalange: key = "middle_proximal_phalange"; break;
        case MiddleIntermediatePhalange: key = "middle_intermediate_phalange"; break;
        case MiddleDistalPhalange: key = "middle_distal_phalange"; break;
        case MiddleFingertip: key = "middle_fingertip"; break;
        case RingProximalPhalange: key = "ring_proximal_phalange"; break;
        case RingIntermediatePhalange: key = "ring_intermediate_phalange"; break;
        case RingDistalPhalange: key = "ring_distal_phalange"; break;
        case RingFingertip: key = "ring_fingertip"; break;
        case PinkyProximalPhalange: key = "pinky_proximal_phalange"; break;
        case PinkyIntermediatePhalange: key = "pinky_intermediate_phalange"; break;
        case PinkyDistalPhalange: key = "pinky_distal_phalange"; break;
        case PinkyFingertip: key = "pinky_fingertip"; break;
    }
    return modifier[key];
}

void initGesture() {
    for (int no{Metacarpals}; no < Last; no++) {
        lastPosition[HandBone(no)] = 0.f;
    }
}

void setGesture(const char* name,
                const std::initializer_list<std::pair<const HandBone, float>>& angles) {
    if (!currGesture.has_value()) {
        currGesture = Gesture{name, float(glfwGetTime()), angles};
        std::cout << "Doing gesture " << name << std::endl;
    }
}

void positionGesture() {
    if (!currGesture.has_value()) {
        return;
    }
    const float currentTime{float(glfwGetTime())};
    float ratio{(currentTime - currGesture->startTime) / gestureDuration};
    for (auto& i : lastPosition) {
        // ignore metacarpals, let hand rotate
        if (i.first == Metacarpals) continue;
        const float old_angle{i.second};
        float new_angle{0.f};
        if (auto iter{currGesture->angles.find(i.first)}; iter != currGesture->angles.end()) {
            new_angle = iter->second;
        }
        getModifier(i.first) =
            glm::rotate(glm::fmat4(1.0f), old_angle + ratio * (new_angle - old_angle),
                        glm::fvec3(0.f, 0.f, 1.f));
        if (ratio > 1) {
            i.second = new_angle;
        }
    }
    if (ratio > 1) {
        currGesture.reset();
        std::cout << "Done" << std::endl;
    }
}

const struct {
    int key;
    const char* name;
    std::initializer_list<std::pair<const HandBone, float>> angles;
} gestures[] = {{GLFW_KEY_C, "clear", {}},
                {GLFW_KEY_0,
                 "0",
                 {{ThumbProximalPhalange, std::numbers::pi / 5},
                  {ThumbIntermediatePhalange, std::numbers::pi / 6},
                  {ThumbDistalPhalange, std::numbers::pi / 2},
                  {IndexProximalPhalange, std::numbers::pi / 2},
                  {IndexIntermediatePhalange, std::numbers::pi / 2},
                  {IndexDistalPhalange, 3 * std::numbers::pi / 7},
                  {MiddleProximalPhalange, std::numbers::pi / 2},
                  {MiddleIntermediatePhalange, std::numbers::pi / 2},
                  {MiddleDistalPhalange, std::numbers::pi / 2},
                  {RingProximalPhalange, std::numbers::pi / 2},
                  {RingIntermediatePhalange, std::numbers::pi / 2},
                  {RingDistalPhalange, 3 * std::numbers::pi / 7},
                  {PinkyProximalPhalange, std::numbers::pi / 2},
                  {PinkyIntermediatePhalange, std::numbers::pi / 2},
                  {PinkyDistalPhalange, 3 * std::numbers::pi / 7}}},
                {GLFW_KEY_1,
                 "1",
                 {{ThumbProximalPhalange, std::numbers::pi / 4},
                  {ThumbDistalPhalange, std::numbers::pi / 3},
                  {IndexIntermediatePhalange, -std::numbers::pi / 8},
                  {MiddleProximalPhalange, std::numbers::pi / 3},
                  {MiddleIntermediatePhalange, std::numbers::pi / 2},
                  {MiddleDistalPhalange, std::numbers::pi / 2},
                  {RingProximalPhalange, std::numbers::pi / 3},
                  {RingIntermediatePhalange, std::numbers::pi / 2},
                  {RingDistalPhalange, 3 * std::numbers::pi / 7},
                  {PinkyProximalPhalange, std::numbers::pi / 3},
                  {PinkyIntermediatePhalange, std::numbers::pi / 2},
                  {PinkyDistalPhalange, 3 * std::numbers::pi / 7}}},
                {GLFW_KEY_2,
                 "2",
                 {{ThumbProximalPhalange, std::numbers::pi / 3},
                  {ThumbIntermediatePhalange, std::numbers::pi / 5},
                  {ThumbDistalPhalange, std::numbers::pi / 3},
                  {IndexIntermediatePhalange, -std::numbers::pi / 8},
                  {MiddleProximalPhalange, std::numbers::pi / 6},
                  {MiddleIntermediatePhalange, -std::numbers::pi / 8},
                  {RingProximalPhalange, std::numbers::pi / 3},
                  {RingIntermediatePhalange, 4 * std::numbers::pi / 7},
                  {RingDistalPhalange, std::numbers::pi / 4},
                  {PinkyProximalPhalange, std::numbers::pi / 3},
                  {PinkyIntermediatePhalange, std::numbers::pi / 2},
                  {PinkyDistalPhalange, std::numbers::pi / 4}}},
                {GLFW_KEY_3,
                 "3",
                 {{ThumbProximalPhalange, std::numbers::pi / 3},
                  {ThumbIntermediatePhalange, std::numbers::pi / 5},
                  {ThumbDistalPhalange, std::numbers::pi / 3},
                  {IndexIntermediatePhalange, -std::numbers::pi / 8},
                  {MiddleIntermediatePhalange, -std::numbers::pi / 8},
                  {RingProximalPhalange, std::numbers::pi / 7},
                  {RingIntermediatePhalange, -std::numbers::pi / 8},
                  {PinkyProximalPhalange, std::numbers::pi / 4},
                  {PinkyIntermediatePhalange, std::numbers::pi / 2},
                  {PinkyDistalPhalange, std::numbers::pi / 4}}},
                {GLFW_KEY_4,
                 "4",
                 {{ThumbProximalPhalange, std::numbers::pi / 3},
                  {ThumbIntermediatePhalange, std::numbers::pi / 4},
                  {ThumbDistalPhalange, std::numbers::pi / 4},
                  {MiddleIntermediatePhalange, -std::numbers::pi / 8},
                  {RingIntermediatePhalange, -std::numbers::pi / 8},
                  {PinkyIntermediatePhalange, -std::numbers::pi / 8}}},
                {GLFW_KEY_5,
                 "5",
                 {{ThumbProximalPhalange, -std::numbers::pi / 8},
                  {IndexIntermediatePhalange, -std::numbers::pi / 8},
                  {MiddleIntermediatePhalange, -std::numbers::pi / 8},
                  {RingIntermediatePhalange, -std::numbers::pi / 8},
                  {PinkyIntermediatePhalange, -std::numbers::pi / 8}}},
                {GLFW_KEY_6,
                 "6",
                 {{ThumbProximalPhalange, -std::numbers::pi / 8},
                  {IndexProximalPhalange, std::numbers::pi / 2},
                  {IndexIntermediatePhalange, std::numbers::pi / 2},
                  {IndexDistalPhalange, std::numbers::pi / 7},
                  {MiddleProximalPhalange, std::numbers::pi / 2},
                  {MiddleIntermediatePhalange, std::numbers::pi / 2},
                  {RingProximalPhalange, std::numbers::pi / 2},
                  {RingIntermediatePhalange, std::numbers::pi / 2},
                  {PinkyProximalPhalange, std::numbers::pi / 5}}},
                {GLFW_KEY_7,
                 "7",
                 {{ThumbProximalPhalange, std::numbers::pi / 3},
                  {ThumbDistalPhalange, -std::numbers::pi / 4},
                  {IndexProximalPhalange, std::numbers::pi / 3},
                  {MiddleProximalPhalange, std::numbers::pi / 3},
                  {RingProximalPhalange, std::numbers::pi / 3},
                  {PinkyProximalPhalange, std::numbers::pi / 3}}},
                {GLFW_KEY_8,
                 "8",
                 {{ThumbProximalPhalange, -std::numbers::pi / 8},
                  {IndexIntermediatePhalange, -std::numbers::pi / 8},
                  {MiddleProximalPhalange, std::numbers::pi / 3},
                  {MiddleIntermediatePhalange, std::numbers::pi / 2},
                  {MiddleDistalPhalange, std::numbers::pi / 2},
                  {RingProximalPhalange, std::numbers::pi / 3},
                  {RingIntermediatePhalange, std::numbers::pi / 2},
                  {RingDistalPhalange, 3 * std::numbers::pi / 7},
                  {PinkyProximalPhalange, std::numbers::pi / 3},
                  {PinkyIntermediatePhalange, std::numbers::pi / 2},
                  {PinkyDistalPhalange, 3 * std::numbers::pi / 7}}},
                {GLFW_KEY_9,
                 "9",
                 {{ThumbProximalPhalange, std::numbers::pi / 4},
                  {ThumbDistalPhalange, std::numbers::pi / 3},
                  {IndexIntermediatePhalange, std::numbers::pi / 3},
                  {IndexDistalPhalange, std::numbers::pi / 3},
                  {MiddleProximalPhalange, std::numbers::pi / 2},
                  {MiddleIntermediatePhalange, std::numbers::pi / 2},
                  {MiddleDistalPhalange, std::numbers::pi / 2},
                  {RingProximalPhalange, std::numbers::pi / 2},
                  {RingIntermediatePhalange, std::numbers::pi / 2},
                  {RingDistalPhalange, 3 * std::numbers::pi / 7},
                  {PinkyProximalPhalange, std::numbers::pi / 2},
                  {PinkyIntermediatePhalange, std::numbers::pi / 2},
                  {PinkyDistalPhalange, 3 * std::numbers::pi / 7}}},
                {GLFW_KEY_T,
                 "thumb",
                 {{ThumbProximalPhalange, -std::numbers::pi / 8},
                  {IndexProximalPhalange, std::numbers::pi / 2},
                  {IndexIntermediatePhalange, std::numbers::pi / 2},
                  {IndexDistalPhalange, std::numbers::pi / 4},
                  {MiddleProximalPhalange, std::numbers::pi / 2},
                  {MiddleIntermediatePhalange, std::numbers::pi / 2},
                  {MiddleDistalPhalange, std::numbers::pi / 2},
                  {RingProximalPhalange, std::numbers::pi / 2},
                  {RingIntermediatePhalange, std::numbers::pi / 2},
                  {RingDistalPhalange, 3 * std::numbers::pi / 7},
                  {PinkyProximalPhalange, std::numbers::pi / 2},
                  {PinkyIntermediatePhalange, std::numbers::pi / 2},
                  {PinkyDistalPhalange, 3 * std::numbers::pi / 7}}},
                {GLFW_KEY_O,
                 "ok",
                 {{ThumbProximalPhalange, std::numbers::pi / 6},
                  {ThumbDistalPhalange, std::numbers::pi / 4},
                  {IndexProximalPhalange, std::numbers::pi / 4},
                  {IndexIntermediatePhalange, std::numbers::pi / 3},
                  {IndexDistalPhalange, std::numbers::pi / 4},
                  {MiddleProximalPhalange, std::numbers::pi / 7},
                  {PinkyIntermediatePhalange, -std::numbers::pi / 8}}}};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_R && action == GLFW_PRESS) enableMetacarpalsRotation ^= true;

    for (const auto& i : gestures) {
        if (i.key == key && action == GLFW_PRESS) {
            setGesture(i.name, i.angles);
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;
    GLuint vertex_shader, fragment_shader, program;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__  // for macos
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(800, 800, "Hand", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK) exit(EXIT_FAILURE);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &SkeletalAnimation::vertex_shader, nullptr);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &SkeletalAnimation::fragment_shader, nullptr);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int linkStatus;
    if (glGetProgramiv(program, GL_LINK_STATUS, &linkStatus), linkStatus == GL_FALSE)
        std::cout << "Error occured in glLinkProgram()" << std::endl;

    auto sr = Scene::loadScene("Hand", "Hand.fbx");
    if (!sr.has_value()) std::cout << "Error occured in loadMesh()" << std::endl;

    sr->get()->setShaderInput(program, "in_position", "in_texcoord", "in_normal", "in_bone_index",
                              "in_bone_weight");

    float passed_time;
    auto metacarpalsRotation{glm::fmat4(1.0f)};

    initGesture();

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        passed_time = clock() / double(CLOCKS_PER_SEC);

        float currentFrame{float(glfwGetTime())};
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processKeyboardInput(window);

        // --- You may edit below ---

        // Example: Rotate the hand
        // * turn around every 4 seconds
        float metacarpals_angle = deltaTime * (std::numbers::pi / 4.0);
        // * target = metacarpals
        // * rotation axis = (1, 0, 0)
        if (enableMetacarpalsRotation) {
            metacarpalsRotation =
                glm::rotate(metacarpalsRotation, metacarpals_angle, glm::fvec3(1.0, 0.0, 0.0));
        }

        getModifier(Metacarpals) = metacarpalsRotation;

        positionGesture();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / float(height);

        glClearColor(0.5, 0.5, 0.5, 1.0);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glm::fmat4 mvp = glm::perspective(glm::radians(fov), ratio, 0.1f, 100.f) *
                         glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp) * modelRotation;
        glUniformMatrix4fv(glGetUniformLocation(program, "u_mvp"), 1, GL_FALSE,
                           (const GLfloat*)&mvp);
        glUniform1i(glGetUniformLocation(program, "u_diffuse"),
                    SCENE_RESOURCE_SHADER_DIFFUSE_CHANNEL);
        Scene::SkeletonTransf bonesTransf;
        sr->get()->getSkeletonTransform(bonesTransf, modifier);
        if (!bonesTransf.empty())
            glUniformMatrix4fv(glGetUniformLocation(program, "u_bone_transf"), bonesTransf.size(),
                               GL_FALSE, (float*)bonesTransf.data());
        sr->get()->render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Scene::unloadScene("Hand");

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}