#ifndef MATERIALS_H
#define MATERIALS_H

#include "common.h"

namespace builtin_materials
{

    struct NaiveMaterial : public common::Material
    {
        NaiveMaterial() {}
        NaiveMaterial(std::shared_ptr<common::ShaderProgram> shader,
                      unsigned int material_id,
                      std::shared_ptr<common::Texture2D> texture)
            : texture(texture), Material(shader, material_id)
        {
        }

        virtual void PrepareForDraw()
        {
            glUseProgram(shader->shader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->texture);
        };

        virtual void Dispose()
        {
            if (shader != nullptr)
            {
                shader->Dispose();
            }
            //TODO texture dispose
        }

        std::shared_ptr<common::Texture2D> texture;
    };

    struct PhongMaterial : public common::Material
    {
        PhongMaterial() {}
        PhongMaterial(std::shared_ptr<common::ShaderProgram> shader,
                      unsigned int material_id,
                      std::shared_ptr<common::Texture2D> diffuse,
                      std::shared_ptr<common::Texture2D> specular,
                      std::shared_ptr<common::Texture2D> normal,
                      float shininess)
            : diffuse(diffuse), specular(specular), normal(normal), shininess(shininess), Material(shader, material_id)
        {
            glUseProgram(shader->shader);
            unsigned int shineLoc = glGetUniformLocation(shader->shader, "shininess");
            glUniform1f(shineLoc, shininess);
            glUniform1i(glGetUniformLocation(shader->shader, "diffuse"), 0);
            glUniform1i(glGetUniformLocation(shader->shader, "specular"), 1);
            glUniform1i(glGetUniformLocation(shader->shader, "normal"), 2);
        }

        virtual void PrepareForDraw()
        {
            glUseProgram(shader->shader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse->texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specular->texture);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, normal->texture);
        };

        virtual void Dispose()
        {
            if (shader != nullptr)
            {
                shader->Dispose();
            }
            //TODO texture dispose
        }

        std::shared_ptr<common::Texture2D> diffuse;
        std::shared_ptr<common::Texture2D> specular;
        std::shared_ptr<common::Texture2D> normal;
        float shininess;
    };

    struct PBRMaterial : public common::Material
    {
        PBRMaterial() {}
        PBRMaterial(std::shared_ptr<common::ShaderProgram> shader,
                    unsigned int material_id,
                    std::shared_ptr<common::Texture2D> albedo,
                    std::shared_ptr<common::Texture2D> metallic,
                    std::shared_ptr<common::Texture2D> roughness,
                    std::shared_ptr<common::Texture2D> normal)
            : albedo(albedo), metallic(metallic), roughness(roughness), normal(normal), Material(shader, material_id)
        {
            glUseProgram(shader->shader);
            glUniform1i(glGetUniformLocation(shader->shader, "albedoMap"), 0);
            glUniform1i(glGetUniformLocation(shader->shader, "metalicMap"), 1);
            glUniform1i(glGetUniformLocation(shader->shader, "roughnessMap"), 2);
            glUniform1i(glGetUniformLocation(shader->shader, "normalMap"), 3);
        }

        virtual void PrepareForDraw()
        {
            glUseProgram(shader->shader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, albedo->texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, metallic->texture);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, roughness->texture);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, normal->texture);
        };

        virtual void Dispose()
        {
            if (shader != nullptr)
            {
                shader->Dispose();
            }
            //TODO texture dispose
        }

        std::shared_ptr<common::Texture2D> albedo;
        std::shared_ptr<common::Texture2D> metallic;
        std::shared_ptr<common::Texture2D> roughness;
        std::shared_ptr<common::Texture2D> normal;
    };
}
#endif