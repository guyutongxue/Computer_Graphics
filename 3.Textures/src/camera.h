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

#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from
// window-system specific input methods
enum class CamMove { Forward, Backward, Left, Right, Up, Down };

// Default camera values
constexpr const float DEFAULT_YAW{-90.0f};
constexpr const float DEFAULT_PITCH{0.0f};
constexpr const float DEFAULT_SPEED{2.5f};
constexpr const float DEFAULT_SENSITIVITY{0.1f};
constexpr const float DEFAULT_ZOOM{45.0f};

// An abstract camera class that processes input and calculates the corresponding Euler Angles,
// Vectors and Matrices for use in OpenGL
class Camera {
public:
    // camera Attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    // euler Angles
    float yaw;
    float pitch;
    // camera options
    float moveSpeed;
    float mouseSensitivity;
    float zoom;

    // constructor with vectors
    Camera(glm::vec3 position = {0.0f, 0.0f, 0.0f}, glm::vec3 up = {0.0f, 1.0f, 0.0f},
           float yaw = DEFAULT_YAW, float pitch = DEFAULT_PITCH);

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 getViewMatrix();

    // processes input received from any keyboard-like input system. Accepts input parameter in the
    // form of camera defined ENUM (to abstract it from windowing systems)
    void processKeyboard(CamMove direction, float deltaTime);

    // processes input received from a mouse input system. Expects the offset value in both the x
    // and y direction.
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical
    // wheel-axis
    void processMouseScroll(float yoffset);

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
};
#endif