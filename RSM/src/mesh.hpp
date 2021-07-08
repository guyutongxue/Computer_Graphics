// Copyright (c) 2021 Guyutongxue
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>  // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <string>
#include <vector>
#include <fstream>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

class Mesh {
public:
    // mesh data
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    GLuint vao;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices)
        : vertices(vertices), indices(indices) {
        // now that we have all the required data, set the vertex buffers and its attribute
        // pointers.
        setupMesh();
    }

    // render the mesh
    void draw(Shader& shader) const {
        // draw mesh
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    // render data
    GLuint vbo, ebo;

    // initializes all the buffer objects/arrays
    void setupMesh() {
        // create buffers/arrays
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly
        // to a glm::vec3/2 array which again translates to 3/2 floats which translates to a byte
        // array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(),
                     GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, position)));
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsetof(Vertex, normal)));

        glBindVertexArray(0);
    }
};
#endif