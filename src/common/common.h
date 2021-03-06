#ifndef COMMON_H
#define COMMON_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include "json.hpp"
#include "../resource/resource.h"
#include "ds.h"

namespace common
{
    enum ShaderType
    {
        VERTEX_SHADER = GL_VERTEX_SHADER,
        FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
        COMPUTE_SHADER = GL_COMPUTE_SHADER
    };

    // TODO layout description
    struct Shader
    {
        unsigned int shader;
        ShaderType type;

        Shader() {}
        Shader(std::string pth, ShaderType type) : type(type)
        {
            std::ifstream f(pth);
            std::stringstream sstream;
            sstream << f.rdbuf();
            std::string code = sstream.str();

            const GLchar *code_p = code.c_str();
            shader = glCreateShader(type);

            glShaderSource(shader, 1, &code_p, NULL);
            glCompileShader(shader);

            int success;
            char infoLog[512];
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::COMPILATION_FAILED\n"
                          << type << " " << infoLog << std::endl;
            }
        }

        void Dispose()
        {
            glDeleteShader(shader);
        }
    };

    struct ShaderProgram : public resources::SerializableObject
    {
        unsigned int shader;

        ShaderProgram() {}
        ShaderProgram(Shader &&vs, Shader &&fs)
        {
            init(std::move(vs), std::move(fs));
        }

        ShaderProgram(Shader &&vs)
        {
            shader = glCreateProgram();
            glAttachShader(shader, vs.shader);
            glLinkProgram(shader);
            int success;
            char infoLog[512];
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                          << infoLog << std::endl;
            }

            vs.Dispose();
        }

        void Dispose()
        {
            glDeleteProgram(shader);
        }

        void init(Shader &&vs, Shader &&fs)
        {
            shader = glCreateProgram();
            glAttachShader(shader, vs.shader);
            glAttachShader(shader, fs.shader);
            glLinkProgram(shader);

            int success;
            char infoLog[512];
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                          << infoLog << std::endl;
            }

            vs.Dispose();
            fs.Dispose();
        }

        virtual void UnserializeJSON(std::string s)
        {
            auto j = nlohmann::json::parse(s);
            std::string vpth = j["vertex"].get<std::string>();
            std::string fpth = j["fragment"].get<std::string>();

            init(Shader(vpth, VERTEX_SHADER), Shader(fpth, FRAGMENT_SHADER));
        }
    };

    struct ComputeShaderProgram
    {
        unsigned int shader;
        ComputeShaderProgram() {}
        ComputeShaderProgram(std::string pth)
        {
            shader = glCreateProgram();
            Shader s(pth, COMPUTE_SHADER);
            glAttachShader(shader, s.shader);
            glLinkProgram(shader);
        }
    };

    struct Texture2D : public resources::SerializableObject
    {
        Texture2D() {}
        Texture2D(std::string pth)
        {
            this->Load(pth);
        }

        virtual void UnserializeJSON(std::string s)
        {
            auto j = nlohmann::json::parse(s);
            unsigned int wraps = j["wraps"].get<unsigned int>();
            unsigned int wrapt = j["wrapt"].get<unsigned int>();
            unsigned int minfilter = j["minfilter"].get<unsigned int>();
            unsigned int magfilter = j["magfilter"].get<unsigned int>();
            unsigned int miplevel = j["miplevel"].get<unsigned int>();
            bool automip = j["automip"].get<bool>();
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapt);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, miplevel - 1);
            if (automip)
            {
                std::string pth = j["paths"][0].get<std::string>();
                loadImage(pth, 0);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                for (int i = 0; i < miplevel; i++)
                {
                    std::string pth = j["paths"][i].get<std::string>();
                    loadImage(pth, i);
                }
            }
        }

        virtual void Load(std::string pth)
        {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            this->loadImage(pth, 0);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        unsigned int texture;

    private:
        void loadImage(std::string &pth, int level)
        {
            int width, height, nrChannels;
            unsigned char *data = stbi_load(pth.c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                GLenum format;
                switch (nrChannels)
                {
                case 1:
                    format = GL_RED;
                    break;
                case 3:
                    format = GL_RGB;
                    break;
                case 4:
                    format = GL_RGBA;
                    break;
                }
                glTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            }
            else
            {
                std::cout << "Failed to load texture" << std::endl;
            }
            stbi_image_free(data);
        }
    };

    struct TextureCube : public resources::SerializableObject
    {
        TextureCube() {}
        TextureCube(std::vector<std::string> &faces)
        {
            init(faces);
        }

        virtual void UnserializeJSON(std::string s)
        {
            auto j = nlohmann::json::parse(s);
            std::vector<std::string> faces(6);
            faces[0] = j["right"].get<std::string>();
            faces[1] = j["left"].get<std::string>();
            faces[2] = j["top"].get<std::string>();
            faces[3] = j["bottom"].get<std::string>();
            faces[4] = j["front"].get<std::string>();
            faces[5] = j["back"].get<std::string>();
            init(faces);
        }

        void init(std::vector<std::string> &faces)
        {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
            int width, height, nrChannels;
            for (int i = 0; i < 6; i++)
            {
                unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
                if (data)
                {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                    stbi_image_free(data);
                }
                else
                {
                    std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
                    stbi_image_free(data);
                }
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }
        unsigned int texture;
    };

    const unsigned int ENGINE_TEXTURE_CNT = 3 + 1;

    struct RenderArguments
    {
        RenderArguments() {}
        RenderArguments(glm::mat4 model) : model(model) {}

        glm::mat4 model;
        BoundingBox box;

        virtual void PrepareForDraw(unsigned int shader_id)
        {
            unsigned int modelLoc = glGetUniformLocation(shader_id, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        }
    };

    struct Material
    {
        Material()
        {
            material_id = render_mode = 0;
        }
        Material(std::shared_ptr<ShaderProgram> shader, unsigned int material_id) : shader(shader), material_id(material_id) {}

        virtual void PrepareForDraw()
        {
            glUseProgram(shader->shader);
        }

        virtual void Dispose() {}

        std::shared_ptr<ShaderProgram> shader;
        unsigned int material_id;
        unsigned int render_mode;
    };

    // TODO layout description
    struct VertexProperties
    {
        float position[3];
        float normal[3];
        float tangent[3];
        float uv[2];
    };

    struct ModelMesh : public resources::SerializableObject
    {
        ModelMesh() {}
        ModelMesh(std::string pth)
        {
            init(pth);
        }

        virtual void Load(std::string pth)
        {
            init(pth);
        }

        void init(std::string pth)
        {
            v_count = id_count = 0;
            std::ifstream f(pth);
            std::string line;
            std::string elem;
            float tmp[8];
            VertexProperties vprop;
            while (std::getline(f, line))
            {
                if (line.length() == 0)
                    break;
                std::istringstream s(line);
                int i = 0;
                while (std::getline(s, elem, ' '))
                {
                    tmp[i] = std::atof(elem.c_str());
                    i++;
                }
                memcpy(&vprop, tmp, 6 * sizeof(float));
                memcpy(&vprop.uv, tmp + 6, 2 * sizeof(float));
                vertices.push_back(vprop);
                v_count++;
            }

            std::getline(f, line);
            std::istringstream s(line);
            while (std::getline(s, elem, ' '))
            {
                indices.push_back(std::atoi(elem.c_str()));
                id_count++;
            }
            f.close();
            box.max = glm::vec3(vertices[0].position[0], vertices[0].position[1], vertices[0].position[2]);
            box.min = box.max;
            for (int i = 0; i < vertices.size(); i++)
                for (int j = 0; j < 3; j++)
                {
                    box.max[j] = std::max(box.max[j], vertices[i].position[j]);
                    box.min[j] = std::min(box.min[j], vertices[i].position[j]);
                }

            for (int i = 0; i < indices.size() / 3; i++)
            {
                // TODO: genbox
                VertexProperties &v1 = vertices[indices[i * 3]];
                VertexProperties &v2 = vertices[indices[i * 3 + 1]];
                VertexProperties &v3 = vertices[indices[i * 3 + 2]];
                glm::vec3 pos1(v1.position[0], v1.position[1], v1.position[2]);
                glm::vec3 pos2(v2.position[0], v2.position[1], v2.position[2]);
                glm::vec3 pos3(v3.position[0], v3.position[1], v3.position[2]);
                glm::vec2 uv1(v1.uv[0], v1.uv[1]);
                glm::vec2 uv2(v2.uv[0], v2.uv[1]);
                glm::vec2 uv3(v3.uv[0], v3.uv[1]);
                glm::vec3 edge1 = pos2 - pos1;
                glm::vec3 edge2 = pos3 - pos1;
                glm::vec2 deltaUV1 = uv2 - uv1;
                glm::vec2 deltaUV2 = uv3 - uv1;
                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                glm::vec3 tangent;
                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                tangent = glm::normalize(tangent);
                for (int j = 0; j < 3; j++)
                {
                    VertexProperties &v = vertices[indices[i * 3 + j]];
                    v.tangent[0] = tangent.x;
                    v.tangent[1] = tangent.y;
                    v.tangent[2] = tangent.z;
                }
            }

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ebo);

            glBindVertexArray(vao);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexProperties), vertices.data(), GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperties), (void *)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperties), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexProperties), (void *)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexProperties), (void *)(9 * sizeof(float)));
            glEnableVertexAttribArray(3);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        void Dispose()
        {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
        }

        void PrepareForDraw()
        {
            glBindVertexArray(vao);
        }

        std::vector<VertexProperties> vertices;

        std::vector<unsigned int> indices;

        int v_count;
        int id_count;
        unsigned int vao;
        unsigned int vbo;
        unsigned int ebo;
        BoundingBox box;
    };

}
#endif