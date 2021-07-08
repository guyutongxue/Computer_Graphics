#version 330 core
layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;

out vec3 fsNormal;
out vec3 fsPosition;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main() {
    fsNormal = mat3(transpose(inverse(model))) * normal;
    vec4 worldPos = model * vec4(position, 1.0);
    fsPosition = worldPos.xyz;
    gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);
}