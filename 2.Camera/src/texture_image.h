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

#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <optional>
#include <filesystem>

using namespace std::string_literals;

class Texture {
private:
    bool available;
    std::string name;
    std::string filename;
    int width;
    int height;
    GLuint tex;


public:
    using NameTextureMap = std::map<std::string, std::shared_ptr<Texture>>;
    static NameTextureMap allTexture;

    Texture();
    Texture(const Texture&) = delete;

    static std::string testAllSuffix(const std::string& no_suffix_name);

    static std::optional<std::shared_ptr<Texture>> loadTexture(const std::string& name);
    static std::optional<std::shared_ptr<Texture>> loadTexture(const std::string& name,
                                                               const std::string& filename);
    static bool unloadTexture(const std::string& name);
    static std::optional<std::shared_ptr<Texture>> getTexture(const std::string& name);

    void clear();
    bool bind(GLenum textureChannel) const;
};