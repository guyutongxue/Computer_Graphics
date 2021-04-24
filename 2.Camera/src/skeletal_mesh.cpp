// Copyright (C) 2021 Guyutongxue
//
// This file is part of CGHomework/Camera.
//
// CGHomework/Camera is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CGHomework/Camera is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CGHomework/Camera.  If not, see <http://www.gnu.org/licenses/>.

// Simple Skeletal Mesh Loader & Renderer
// Original Author: Yi Kangrui <yikangrui@pku.edu.cn>

#include "skeletal_mesh.h"

ParametricVertex::ParametricVertex() : position{}, texcoord{}, normal{}, boneId{}, boneWeight{} {}

ParametricVertex::ParametricVertex(const aiVector3D& p, const aiVector2D& tc, const aiVector3D& n)
    : position{p.x, p.y, p.z},
      texcoord{tc.x, tc.y},
      normal{n.x, n.y, n.z},
      boneId{},
      boneWeight{} {}

bool ParametricVertex::addBone(unsigned int id, float weight) {
    if (weight < 1e-6f) return false;
    int min_weight_index{0};
    for (int i = 1; i < SCENE_RESOURCE_BONE_PER_VERTEX; i++) {
        if (boneWeight[i] < boneWeight[min_weight_index]) min_weight_index = i;
    }
    if (boneWeight[min_weight_index] < weight) {
        boneId[min_weight_index] = id;
        boneWeight[min_weight_index] = weight;
        return true;
    }
    return false;
}

Material::Material() : diffuse(std::nullopt) {}

bool Material::setDiffuse(const std::string& name, const std::string& filename) {
    return (diffuse = Texture::loadTexture(name, filename)).has_value();
}

Bone::Bone(const aiMatrix4x4& m) : localTransf(m) {}

Scene::Scene() : available{false}, vao{0}, vbo{0}, ebo{0} {}

Scene::~Scene() {
    clear();
}

void Scene::clear() {
    available = false;
    name = ""s;
    filename = ""s;
    // importer..
    scene = nullptr;
    glDeleteVertexArrays(1, &vao);
    vao = 0;
    glDeleteBuffers(1, &vbo);
    vbo = 0;
    glDeleteBuffers(1, &ebo);
    ebo = 0;
    meshEntry.clear();
    material.clear();
    skeleton.clear();
    nameBoneMap.clear();
}

std::string Scene::testAllSuffix(const std::string& no_suffix_name) {
    for (const auto& i : {".obj", ".dae", ".fbx"}) {
        std::string try_filename{no_suffix_name + i};
        if (std::filesystem::exists(try_filename)) {
            return try_filename;
        }
    }
    return ""s;
}

Scene::NameSceneMap Scene::allScene;

std::optional<std::shared_ptr<Scene>> Scene::loadScene(const std::string& name) {
    if (std::string filename{testAllSuffix(name)}; !filename.empty())
        return loadScene(name, filename);
    return std::nullopt;
}

std::optional<std::shared_ptr<Scene>> Scene::loadScene(const std::string& name,
                                                       const std::string& filename) {
    if (!std::filesystem::exists(filename)) {
        std::cerr << "loadScene: filename " << filename << " doesn't exists" << std::endl;
        return std::nullopt;
    }

    auto insertion{allScene.insert({name, std::make_shared<Scene>()})};
    auto& target{insertion.first->second};
    if (!insertion.second) {
        // insert fail
        if (target->filename == filename && target->available) {
            return target;
        } else {
            target->clear();
        }
    }

    target->name = name;
    target->filename = filename;

    target->scene = target->importer.ReadFile(
        filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                      aiProcess_JoinIdenticalVertices);
    if (!target->scene) return std::nullopt;

    std::vector<ParametricVertex> vertexAssembly;
    std::vector<unsigned int> indexAssembly;

    int nTotalMeshes = target->scene->mNumMeshes;
    target->meshEntry.resize(nTotalMeshes);

    int nTotalVertices = 0;
    int nTotalIndices = 0;
    for (int i = 0; i < nTotalMeshes; i++) {
        const aiMesh* curMesh = target->scene->mMeshes[i];
        int nMeshVertices = curMesh->mNumVertices;
        int nMeshBones = curMesh->mNumBones;
        int nMeshFaces = curMesh->mNumFaces;

        target->meshEntry[i].facetCornerNum = nMeshFaces * 3;
        target->meshEntry[i].indexOffset = nTotalIndices;
        target->meshEntry[i].vertexOffset = nTotalVertices;
        target->meshEntry[i].materialIndex = curMesh->mMaterialIndex;

        nTotalVertices += nMeshVertices;
        nTotalIndices += nMeshFaces * 3;

        for (int j = 0; j < nMeshVertices; j++) {
            aiVector2D curTexcoord(.0f, .0f);
            if (curMesh->HasTextureCoords(0))
                curTexcoord =
                    aiVector2D(curMesh->mTextureCoords[0][j].x, curMesh->mTextureCoords[0][j].y);
            vertexAssembly.push_back(
                ParametricVertex(curMesh->mVertices[j], curTexcoord, curMesh->mNormals[j]));
        }
        for (int j = 0; j < nMeshBones; j++) {
            std::string boneName = curMesh->mBones[j]->mName.data;
            std::pair<std::map<std::string, unsigned int>::iterator, bool> insertResult;
            insertResult =
                target->nameBoneMap.insert(std::make_pair(boneName, target->skeleton.size()));
            if (insertResult.second) {
                target->skeleton.push_back(Bone(curMesh->mBones[j]->mOffsetMatrix));
                int nBoneVertexWeight = curMesh->mBones[j]->mNumWeights;
                for (int k = 0; k < nBoneVertexWeight; k++) {
                    int vertexId = target->meshEntry[i].vertexOffset +
                                   curMesh->mBones[j]->mWeights[k].mVertexId;
                    float weight = curMesh->mBones[j]->mWeights[k].mWeight;
                    vertexAssembly[vertexId].addBone(insertResult.first->second, weight);
                }
            }
        }
        for (int j = 0; j < nMeshFaces; j++) {
            for (int k = 0; k < 3; k++) indexAssembly.push_back(curMesh->mFaces[j].mIndices[k]);
        }
    }

    std::string filepath_prefix;
    {
        size_t slashpos = filename.rfind('/');
        size_t conslashpos = filename.rfind('\\');
        if (conslashpos != std::string::npos) {
            if (slashpos == std::string::npos || slashpos < conslashpos) slashpos = conslashpos;
        }
        if (slashpos != std::string::npos) {
            filepath_prefix = filename.substr(0, slashpos + 1);
        }
    }
    int nTotalMaterials = target->scene->mNumMaterials;
    target->material.resize(nTotalMaterials);
    for (int i = 0; i < nTotalMaterials; i++) {
        const aiMaterial* curMaterial = target->scene->mMaterials[i];

        if (curMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString ai_filepath;
            if (curMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &ai_filepath, NULL, NULL, NULL,
                                        NULL, NULL) == AI_SUCCESS) {
                std::string filepath(filepath_prefix + ai_filepath.data);
                std::string dirpath, filename;
                size_t slashpos = filepath.rfind('/');
                size_t conslashpos = filepath.rfind('\\');
                if (conslashpos != std::string::npos) {
                    if (slashpos == std::string::npos || slashpos < conslashpos)
                        slashpos = conslashpos;
                }
                if (slashpos != std::string::npos) {
                    dirpath = filepath.substr(0, slashpos + 1);
                    filename = filepath.substr(slashpos + 1, std::string::npos);
                } else {
                    dirpath = std::string();
                    filename = filepath;
                }
                if (!target->material[i].setDiffuse(filename, dirpath + filename))
                    std::cout << "Error loading diffuse " << filepath << std::endl;
            }
        }
    }

    glGenVertexArrays(1, &target->vao);
    glBindVertexArray(target->vao);

    glGenBuffers(1, &target->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, target->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ParametricVertex) * vertexAssembly.size(),
                 vertexAssembly.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &target->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, target->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexAssembly.size(),
                 indexAssembly.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    target->available = true;
    return target;
}

bool Scene::unloadScene(const std::string& name) {
    return allScene.erase(name) != 0;
}

std::optional<std::shared_ptr<Scene>> Scene::getScene(const std::string& name) {
    if (auto find_result{allScene.find(name)}; find_result != allScene.end())
        return find_result->second;
    return std::nullopt;
}

void Scene::recursivelyGetTransf(SkeletonTransf& skTransf, SkeletonModifier& modifier, aiNode* node,
                                 aiMatrix4x4 parentTransf, const aiMatrix4x4& invTransf) const {
    aiMatrix4x4 globalTransf = parentTransf * node->mTransformation;
    auto boneFound = nameBoneMap.find(std::string(node->mName.data));
    if (boneFound != nameBoneMap.end()) {
        SkeletonModifier::const_iterator boneModFound =
            modifier.find(std::string(node->mName.data));
        aiMatrix4x4 boneMod;
        if (boneModFound != modifier.end()) {
            auto t = glm::transpose(boneModFound->second);
            memcpy(&boneMod, &t, 16 * sizeof(float));
            globalTransf *= boneMod;
        }
        aiMatrix4x4 finalMtrx = invTransf * globalTransf * skeleton[boneFound->second].localTransf;
        memcpy(&skTransf[boneFound->second], &finalMtrx.Transpose(),
               sizeof(skTransf[boneFound->second]));
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        recursivelyGetTransf(skTransf, modifier, node->mChildren[i], globalTransf, invTransf);
    }
}

bool Scene::getSkeletonTransform(SkeletonTransf& transf, SkeletonModifier& modifier) const {
    if (!available) return false;

    transf.resize(skeleton.size());

    aiMatrix4x4 identityMtrx, invTransf = scene->mRootNode->mTransformation;
    invTransf.Inverse();
    recursivelyGetTransf(transf, modifier, scene->mRootNode, identityMtrx, invTransf);
    return !transf.empty();
}

bool Scene::setShaderInput(GLuint program, const std::string& posiName, const std::string& texcName,
                           const std::string& normName, const std::string& bnidName,
                           const std::string& bnwtName) {
    if (!available) return false;

    ParametricVertex example;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    {
        GLint posiLoc = glGetAttribLocation(program, posiName.c_str());
        if (posiLoc >= 0) {
            glEnableVertexAttribArray(posiLoc);
            glVertexAttribPointer(posiLoc, 3, GL_FLOAT, GL_FALSE, sizeof(ParametricVertex),
                                  (const void*)((char*)example.position - (char*)&example));
        }
    }
    {
        GLint texcLoc = glGetAttribLocation(program, texcName.c_str());
        if (texcLoc >= 0) {
            glEnableVertexAttribArray(texcLoc);
            glVertexAttribPointer(texcLoc, 2, GL_FLOAT, GL_FALSE, sizeof(ParametricVertex),
                                  (const void*)((char*)example.texcoord - (char*)&example));
        }
    }
    {
        GLint normLoc = glGetAttribLocation(program, normName.c_str());
        if (normLoc >= 0) {
            glEnableVertexAttribArray(normLoc);
            glVertexAttribPointer(normLoc, 3, GL_FLOAT, GL_FALSE, sizeof(ParametricVertex),
                                  (const void*)((char*)example.normal - (char*)&example));
        }
    }
    {
        GLint bnidLoc = glGetAttribLocation(program, bnidName.c_str());
        if (bnidLoc >= 0) {
            glEnableVertexAttribArray(bnidLoc);
            glVertexAttribIPointer(bnidLoc, SCENE_RESOURCE_BONE_PER_VERTEX, GL_INT,
                                   sizeof(ParametricVertex),
                                   (const void*)((char*)example.boneId - (char*)&example));
        }
    }
    {
        GLint bnwtLoc = glGetAttribLocation(program, bnwtName.c_str());
        if (bnwtLoc >= 0) {
            glEnableVertexAttribArray(bnwtLoc);
            glVertexAttribPointer(bnwtLoc, SCENE_RESOURCE_BONE_PER_VERTEX, GL_FLOAT, GL_FALSE,
                                  sizeof(ParametricVertex),
                                  (const void*)((char*)example.boneWeight - (char*)&example));
        }
    }

    glBindVertexArray(0);

    return true;
}

void Scene::render() const {
    if (!available) return;
    glBindVertexArray(vao);
    for (int i = 0; i < meshEntry.size(); i++) {
        auto a = material[meshEntry[i].materialIndex].diffuse;
        if (!a.has_value() || !a->get()->bind(SCENE_RESOURCE_SHADER_DIFFUSE_CHANNEL))
            glBindTexture(GL_TEXTURE_2D, 0);

        glDrawElementsBaseVertex(GL_TRIANGLES, meshEntry[i].facetCornerNum, GL_UNSIGNED_INT,
                                 (void*)(sizeof(unsigned int) * meshEntry[i].indexOffset),
                                 meshEntry[i].vertexOffset);
    }
    glBindVertexArray(0);
}