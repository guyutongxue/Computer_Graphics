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

#include "texture_image.h"

Texture::Texture() : available{false}, name{}, filename{}, width{0}, height{0}, tex{0u} {}

Texture::NameTextureMap Texture::allTexture{};

std::string Texture::testAllSuffix(const std::string& no_suffix_name) {
    for (const auto& i : {".bmp", ".jpg", ".png", ".tga"}) {
        std::string try_filename{no_suffix_name + i};
        if (std::filesystem::exists(try_filename)) {
            return try_filename;
        }
    }
    return ""s;
}

std::optional<std::shared_ptr<Texture>> Texture::loadTexture(const std::string& name) {
    if (std::string filename{testAllSuffix(name)}; !filename.empty())
        return loadTexture(name, filename);
    return std::nullopt;
}

std::optional<std::shared_ptr<Texture>> Texture::loadTexture(const std::string& name, const std::string& filename) {
    if (GLenum gl_error_code{GL_NO_ERROR}; (gl_error_code = glGetError()) != GL_NO_ERROR) {
        std::cerr << "ERROR before loadTexture: \n" << gluErrorString(gl_error_code) << std::endl;
    }
    std::cout << name << "<>" << filename << std::endl;
    if (!std::filesystem::exists(filename)) {
        std::cerr << "loadTexture: filename " << filename << " doesn't exists" << std::endl;
        return std::nullopt;
    }
    auto insertion{allTexture.insert({name, std::make_shared<Texture>()})};
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

    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    unsigned char* data{stbi_load(filename.c_str(), &width, &height, &channels, 0)};
    if (!data) {
        std::cerr << "loadTexture: stb_load " << filename << "fail" << std::endl;
        return std::nullopt;
    }
    target->width = width;
    target->height = height;

    GLenum format{GL_RGBA};
    if (channels == 1)
        format = GL_R;
    if (channels == 2)
        format = GL_RG;
    if (channels == 3)
        format = GL_RGB;
    glGenTextures(1, &(target->tex));
    glBindTexture(GL_TEXTURE_2D, target->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, format,
                 target->width, target->height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    if (GLenum gl_error_code{GL_NO_ERROR}; (gl_error_code = glGetError()) != GL_NO_ERROR) {
        std::cerr << "ERROR in loadTexture: \n" << gluErrorString(gl_error_code) << std::endl;
        return std::nullopt;
    }
    target->available = true;
    return target;
}

bool Texture::unloadTexture(const std::string& name) {
    return allTexture.erase(name) != 0;
}

std::optional<std::shared_ptr<Texture>> Texture::getTexture(const std::string& name) {
    if (auto result{allTexture.find(name)}; result != allTexture.end()) {
        return result->second;
    }
    return std::nullopt;
}

void Texture::clear() {
    available = false;
    name = ""s;
    filename = ""s;
    glDeleteTextures(1, &tex);
    tex = 0;
}

bool Texture::bind(GLenum textureChannel) const {
    if (!available)
        return false;
    glActiveTexture(GL_TEXTURE0 + textureChannel);
    glBindTexture(GL_TEXTURE_2D, tex);
    return true;
}
