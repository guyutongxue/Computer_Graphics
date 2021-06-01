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

#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : front{0.0f, 0.0f, -1.0f},
      moveSpeed{DEFAULT_SPEED},
      mouseSensitivity{DEFAULT_SENSITIVITY},
      zoom{DEFAULT_ZOOM},
      position{position},
      worldUp{up},
      yaw{yaw},
      pitch{pitch} {
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(CamMove direction, float deltaTime) {
    float velocity = moveSpeed * deltaTime;
    switch (direction) {
        case CamMove::Forward: position += front * velocity; break;
        case CamMove::Backward: position -= front * velocity; break;
        case CamMove::Left: position -= right * velocity; break;
        case CamMove::Right: position += right * velocity; break;
        case CamMove::Up: position += up * velocity; break;
        case CamMove::Down: position -= up * velocity; break;
    }
}

void Camera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    zoom -= (float)yoffset;
    if (zoom < 1.0f) zoom = 1.0f;
    if (zoom > 45.0f) zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    // calculate the new Front vector
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    // also re-calculate the right and up vector
    // normalize the vectors, because their length gets closer to 0 the
    // more you look up or down which results in slower movement.
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}