#version 330 core
layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;

out vec3 fsNormal;
out vec3 fsPosition;
out vec4 fsLightSpacePosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0f);
    fsNormal = mat3(transpose(inverse(model))) * normal;
    fsPosition = vec3(model * vec4(position, 1.0));
    fsLightSpacePosition = lightSpaceMatrix * vec4(fsPosition, 1.0);
}