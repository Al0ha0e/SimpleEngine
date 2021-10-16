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

namespace common
{

    enum ShaderType
    {
        VERTEX_SHADER = GL_VERTEX_SHADER,
        FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
        COMPUTE_SHADER = GL_COMPUTE_SHADER
    };

    //TODO layout description
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

    struct ShaderProgram
    {
        unsigned int shader;

        ShaderProgram() {}
        ShaderProgram(Shader &&vs, Shader &&fs)
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

        void Dispose()
        {
            glDeleteProgram(shader);
        }
    };

    struct Texture
    {
        Texture() {}
        Texture(std::string pth)
        {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            int width, height, nrChannels;
            unsigned char *data = stbi_load(pth.c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                std::cout << "Failed to load texture" << std::endl;
            }
            stbi_image_free(data);
        }
        unsigned int texture;
    };

    struct RenderArguments
    {

        RenderArguments() {}
        RenderArguments(glm::mat4 model,
                        glm::mat4 view,
                        glm::mat4 projection) : model(model), view(view), projection(projection) {}

        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    struct Material
    {
        Material() {}
        Material(std::shared_ptr<ShaderProgram> shader,
                 std::shared_ptr<RenderArguments> args) : shader(shader), args(args)
        {
            //TODO call methods in Shader class to set these matrices
            glUseProgram(shader->shader);
            unsigned int id = shader->shader;
            modelLoc = glGetUniformLocation(id, "model");
            viewLoc = glGetUniformLocation(id, "view");
            projectionLoc = glGetUniformLocation(id, "projection");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(args->model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(args->view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(args->projection));
        }

        void UpdateM(glm::mat4 model)
        {
            glUseProgram(shader->shader);
            args->model = model;
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        }

        void UpdateV(glm::mat4 view)
        {
            glUseProgram(shader->shader);
            args->view = view;
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        }

        void UpdateP(glm::mat4 projection)
        {
            glUseProgram(shader->shader);
            args->projection = projection;
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }

        virtual void PrepareForDraw()
        {
            glUseProgram(shader->shader);
        }

        virtual void Dispose() {}

        std::shared_ptr<ShaderProgram> shader;
        std::shared_ptr<RenderArguments> args;
        int modelLoc, viewLoc, projectionLoc;
    };

    //TODO layout description
    struct ModelMesh
    {
        ModelMesh() {}
        ModelMesh(std::string pth)
        {
            v_count = id_count = 0;
            std::ifstream f(pth);
            std::string line;
            std::string elem;
            while (std::getline(f, line))
            {
                if (line.length() == 0)
                    break;
                std::istringstream s(line);
                while (std::getline(s, elem, ' '))
                    vertices.push_back(std::atof(elem.c_str()));
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

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ebo);

            glBindVertexArray(vao);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

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

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        int v_count;
        int id_count;
        unsigned int vao;
        unsigned int vbo;
        unsigned int ebo;
    };

}
#endif