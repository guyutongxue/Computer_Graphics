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
#include <ctime>

#ifndef M_PI
#define M_PI (3.1415926535897932)
#endif

#include <glm\gtc\matrix_transform.hpp>
#include <iostream>

#include "skeletal_mesh.h"

namespace SkeletalAnimation {
const char* vertex_shader_450 =
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

const char* fragment_shader_450 =
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

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
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

    window = glfwCreateWindow(800, 800, "OpenGL output", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK) exit(EXIT_FAILURE);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &SkeletalAnimation::vertex_shader_450, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &SkeletalAnimation::fragment_shader_450, NULL);
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
    SkeletonModifier modifier;

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window)) {
        passed_time = clock() / double(CLOCKS_PER_SEC);

        // --- You may edit below ---

        // Example: Rotate the hand
        // * turn around every 4 seconds
        float metacarpals_angle = passed_time * (M_PI / 4.0);
        // * target = metacarpals
        // * rotation axis = (1, 0, 0)
        modifier["metacarpals"] =
            glm::rotate(glm::fmat4(1.0f), metacarpals_angle, glm::fvec3(1.0, 0.0, 0.0));

        /**********************************************************************************\
        *
        * To animate fingers, modify modifier["HAND_SECTION"] each frame,
        * where HAND_SECTION can only be one of the bone names in the Hand's Hierarchy.
        *
        * A virtual hand's structure is like this: (slightly DIFFERENT from the real world)
        *    5432 1
        *    ....        1 = thumb           . = fingertip
        *    |||| .      2 = index finger    | = distal phalange
        *    $$$$ |      3 = middle finger   $ = intermediate phalange
        *    #### $      4 = ring finger     # = proximal phalange
        *    OOOO#       5 = pinky           O = metacarpals
        *     OOO
        * (Hand in the real world -> https://en.wikipedia.org/wiki/Hand)
        *
        * From the structure we can infer the Hand's Hierarchy:
        *	- metacarpals
        *		- thumb_proximal_phalange
        *			- thumb_intermediate_phalange
        *				- thumb_distal_phalange
        *					- thumb_fingertip
        *		- index_proximal_phalange
        *			- index_intermediate_phalange
        *				- index_distal_phalange
        *					- index_fingertip
        *		- middle_proximal_phalange
        *			- middle_intermediate_phalange
        *				- middle_distal_phalange
        *					- middle_fingertip
        *		- ring_proximal_phalange
        *			- ring_intermediate_phalange
        *				- ring_distal_phalange
        *					- ring_fingertip
        *		- pinky_proximal_phalange
        *			- pinky_intermediate_phalange
        *				- pinky_distal_phalange
        *					- pinky_fingertip
        *
        * Notice that modifier["HAND_SECTION"] is a local transformation matrix,
        * where (1, 0, 0) is the bone's direction, and apparently (0, 1, 0) / (0, 0, 1)
        * is perpendicular to the bone.
        * Particularly, (0, 0, 1) is the rotation axis of the nearer joint.
        *
        \**********************************************************************************/

        // Example: Animate the index finger
        // * period = 2.4 seconds
        float period = 2.4f;
        float time_in_period = fmod(passed_time, period);
        // * angle: 0 -> PI/3 -> 0
        float thumb_angle = abs(time_in_period / (period * 0.5f) - 1.0f) * (M_PI / 3.0);
        // * target = proximal phalange of the index
        // * rotation axis = (0, 0, 1)
        modifier["index_proximal_phalange"] =
            glm::rotate(glm::fmat4(1.0f), thumb_angle, glm::fvec3(0.0, 0.0, 1.0));

        // --- You may edit above ---

        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glClearColor(0.5, 0.5, 0.5, 1.0);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glm::fmat4 mvp = glm::ortho(-12.5f * ratio, 12.5f * ratio, -5.f, 20.f, -20.f, 20.f) *
                         glm::lookAt(glm::fvec3(.0f, .0f, -1.f), glm::fvec3(.0f, .0f, .0f),
                                     glm::fvec3(.0f, 1.f, .0f));
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