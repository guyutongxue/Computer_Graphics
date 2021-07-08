// Copyright (c) 2021 Guyutongxue
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.h"
#include "mesh.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

class Model {
public:
    // model data
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(const std::string& path, bool gamma = false) : gammaCorrection{gamma} {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void draw(Shader& shader) {
        shader.use();

        // translate to our scene, then rotate 1r/4s
        auto model{glm::translate(glm::mat4(1.0f), {-7.0f, 0.f, 7.f})};
        shader.setUniform(
            "model", glm::rotate(model, static_cast<float>(glfwGetTime() * std::numbers::pi / 4.0),
                                 glm::vec3(0.0, 1.0, 0.0)));

        shader.setUniform("material.diffuse", 0.8f, 0.8f, 0.8f);
        for (const auto& i : meshes) i.draw(shader);
    }

private:

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in
    // the meshes vector.
    void loadModel(const std::string& path) {
        // read file via ASSIMP
        Assimp::Importer importer;
        auto scene{importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                               aiProcess_FlipUVs | aiProcess_CalcTangentSpace)};
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
            !scene->mRootNode) {  // if is Not Zero
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node
    // and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene) {
        // process each mesh located at the current node
        for (auto i{0uz}; i < node->mNumMeshes; i++) {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations
            // between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the
        // children nodes
        for (auto i{0uz}; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;

        // walk through each of the mesh's vertices
        for (auto i{0uz}; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
            if (mesh->HasNormals()) {
                vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
            }
            vertices.push_back(vertex);
        }

        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the
        // corresponding vertex indices.
        for (auto i{0uz}; i < mesh->mNumFaces; i++) {
            auto face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (auto j{0uz}; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // do not need to process materials

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices);
    }
};

#endif