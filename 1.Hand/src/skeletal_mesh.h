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


#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "texture_image.h"

#define SCENE_RESOURCE_SHADER_POSI_LOCATION 0
#define SCENE_RESOURCE_SHADER_TEXC_LOCATION 1
#define SCENE_RESOURCE_SHADER_NORM_LOCATION 2
#define SCENE_RESOURCE_SHADER_BONE_LOCATION 3
#define SCENE_RESOURCE_SHADER_BNWT_LOCATION 4

#define SCENE_RESOURCE_SHADER_DIFFUSE_CHANNEL 0

#define SCENE_RESOURCE_BONE_PER_VERTEX 4

typedef std::map<std::string, glm::fmat4> SkeletonModifier;

struct ParametricVertex {
    float position[3];
    float texcoord[2];
    float normal[3];
    unsigned int boneId[SCENE_RESOURCE_BONE_PER_VERTEX];
    float boneWeight[SCENE_RESOURCE_BONE_PER_VERTEX];

    ParametricVertex();
    ParametricVertex(const aiVector3D& position, const aiVector2D& texcoord,
                     const aiVector3D& normal);

    bool addBone(unsigned int id, float weight);
};

struct MeshEntry {
    unsigned int facetCornerNum;
    unsigned int indexOffset;
    unsigned int vertexOffset;
    unsigned int materialIndex;
};

struct Material {
    std::optional<std::shared_ptr<const Texture>> diffuse;
    Material();
    bool setDiffuse(const std::string& name, const std::string& filename = ""s);
};

struct Bone {
    aiMatrix4x4 localTransf;

    Bone(const aiMatrix4x4& _m);
};

class Scene {
public:
    using NameSceneMap = std::map<std::string, std::shared_ptr<Scene>>;
    using SkeletonTransf = std::vector<glm::fmat4>;
    using NameBoneMap = std::map<std::string, unsigned int>;
    static NameSceneMap allScene;

    Scene(const Scene&) = delete;
    Scene();
    ~Scene();

private:
    bool available;
    std::string name;
    std::string filename;
    Assimp::Importer importer;
    const aiScene* scene;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    std::vector<MeshEntry> meshEntry;
    std::vector<Material> material;
    std::vector<Bone> skeleton;
    NameBoneMap nameBoneMap;

public:
    void clear();

    static std::string testAllSuffix(const std::string& no_suffix_name);

    static std::optional<std::shared_ptr<Scene>> loadScene(const std::string& name);
    static std::optional<std::shared_ptr<Scene>> loadScene(const std::string& name,
                                                           const std::string& filename);

    static bool unloadScene(const std::string& name);

    static std::optional<std::shared_ptr<Scene>> getScene(const std::string& name);

    void recursivelyGetTransf(SkeletonTransf& skTransf, SkeletonModifier& modifier, aiNode* node,
                              aiMatrix4x4 parentTransf, const aiMatrix4x4& invTransf) const;

    bool getSkeletonTransform(SkeletonTransf& transf, SkeletonModifier& modifier) const;

    bool setShaderInput(GLuint program, const std::string& posiName, const std::string& texcName,
                        const std::string& normName, const std::string& bnidName,
                        const std::string& bnwtName);

    void render() const;
};