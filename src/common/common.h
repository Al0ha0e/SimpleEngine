#ifndef COMMON_H
#define COMMON_H

#include "../render/renderer.h"

#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace common
{

    enum ShaderType
    {
        VERTEX_SHADER = GL_VERTEX_SHADER,
        FRAGMENT_SHADER = GL_FRAGMENT_SHADER,
        COMPUTE_SHADER = GL_COMPUTE_SHADER
    };

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

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }

        void Dispose()
        {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
        }

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        int v_count;
        int id_count;
        unsigned int vao;
        unsigned int vbo;
        unsigned int ebo;
    };

    class GameObject
    {
    public:
        GameObject() {}

        GameObject(std::shared_ptr<renderer::Renderer> rd,
                   std::shared_ptr<ShaderProgram> shader,
                   std::shared_ptr<ModelMesh> mesh) : rd(rd),
                                                      shader(shader),
                                                      mesh(mesh) {}

        virtual void OnStart()
        {
            rd->GetRenderID(renderer::OPAQUE);
            renderer::RenderQueueItem item(mesh->vao, shader->shader, mesh->id_count);
            rd->InsertToQueue(render_id, item, renderer::OPAQUE);
        }
        virtual void BeforeRender() {}
        virtual void AfterRender() {}

        virtual void Dispose()
        {
            if (mesh != nullptr)
                mesh->Dispose();
            if (shader != nullptr)
                shader->Dispose();
        }

    private:
        std::shared_ptr<renderer::Renderer> rd;
        std::shared_ptr<ShaderProgram> shader;
        std::shared_ptr<ModelMesh> mesh;
        int render_id;
    };
}
#endif