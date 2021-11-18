#ifndef MATERIALS_H
#define MATERIALS_H

#include <json.hpp>

#include "common.h"
#include "../resource/resource.h"

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

    struct ParallaxPhongMaterial : public common::Material
    {
        ParallaxPhongMaterial() {}
        ParallaxPhongMaterial(std::shared_ptr<common::ShaderProgram> shader,
                              unsigned int material_id,
                              std::shared_ptr<common::Texture2D> diffuse,
                              std::shared_ptr<common::Texture2D> specular,
                              std::shared_ptr<common::Texture2D> normal,
                              std::shared_ptr<common::Texture2D> depth,
                              float shininess,
                              float height_scale)
            : diffuse(diffuse), specular(specular), normal(normal), depth(depth), shininess(shininess), height_scale(height_scale), Material(shader, material_id)
        {
            glUseProgram(shader->shader);
            unsigned int shineLoc = glGetUniformLocation(shader->shader, "shininess");
            glUniform1f(shineLoc, shininess);
            unsigned int heightLoc = glGetUniformLocation(shader->shader, "height_scale");
            glUniform1f(heightLoc, height_scale);
            glUniform1i(glGetUniformLocation(shader->shader, "diffuse"), 0);
            glUniform1i(glGetUniformLocation(shader->shader, "specular"), 1);
            glUniform1i(glGetUniformLocation(shader->shader, "normal"), 2);
            glUniform1i(glGetUniformLocation(shader->shader, "depth"), 3);
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
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, depth->texture);
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
        std::shared_ptr<common::Texture2D> depth;
        float shininess;
        float height_scale;
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

    struct ParallaxPBRMaterial : public common::Material
    {
        ParallaxPBRMaterial() {}
        ParallaxPBRMaterial(std::shared_ptr<common::ShaderProgram> shader,
                            unsigned int material_id,
                            std::shared_ptr<common::Texture2D> albedo,
                            std::shared_ptr<common::Texture2D> metallic,
                            std::shared_ptr<common::Texture2D> roughness,
                            std::shared_ptr<common::Texture2D> normal,
                            std::shared_ptr<common::Texture2D> depth,
                            float height_scale)
            : albedo(albedo), metallic(metallic), roughness(roughness),
              normal(normal), depth(depth), height_scale(height_scale), Material(shader, material_id)
        {
            glUseProgram(shader->shader);
            unsigned int heightLoc = glGetUniformLocation(shader->shader, "height_scale");
            glUniform1f(heightLoc, height_scale);
            glUniform1i(glGetUniformLocation(shader->shader, "albedoMap"), 0);
            glUniform1i(glGetUniformLocation(shader->shader, "metalicMap"), 1);
            glUniform1i(glGetUniformLocation(shader->shader, "roughnessMap"), 2);
            glUniform1i(glGetUniformLocation(shader->shader, "normalMap"), 3);
            glUniform1i(glGetUniformLocation(shader->shader, "depthMap"), 4);
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
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, depth->texture);
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
        std::shared_ptr<common::Texture2D> depth;
        float height_scale;
    };

    typedef std::pair<std::string, int> kv_int;
    typedef std::pair<std::string, float> kv_float;
    typedef std::pair<std::string, std::shared_ptr<common::Texture2D>> kv_tex2d;
    typedef std::pair<std::string, std::shared_ptr<common::TextureCube>> kv_texcube;

    struct CustomMaterial : public common::Material, public resources::SerializableObject
    {
        CustomMaterial() {}
        CustomMaterial(std::shared_ptr<common::ShaderProgram> shader, unsigned int material_id) : Material(shader, material_id)
        {
        }

        virtual void Dispose()
        {
            if (shader != nullptr)
            {
                shader->Dispose();
            }
            //TODO texture dispose
        }

        virtual void UnserializeJSON(std::string s, resources::ResourceManager *manager)
        {
            auto j = nlohmann::json::parse(s);
            std::string shaderpth = j["shader"].get<std::string>();
            shader = manager->LoadMeta<common::ShaderProgram>(shaderpth);
            glUseProgram(shader->shader);
            auto tx2d = j["2D_textures"];
            auto txcube = j["cube_textures"];
            auto floatvals = j["float_vals"];
            auto intvals = j["int_vals"];
            int texture_cnt = 0;
            for (auto &texture_info : tx2d)
            {
                std::string name = texture_info["name"].get<std::string>();
                auto tex = manager->Load<common::Texture2D>(texture_info["path"].get<std::string>());
                textures_2d.insert(kv_tex2d(name, tex));
                idxs.insert(kv_int(name, texture_cnt));
                glUniform1i(glGetUniformLocation(shader->shader, name.c_str()), texture_cnt);
                texture_cnt++;
            }
            // for (auto &texture_info : txcube)
            // {
            //     auto tex = manager->LoadMeta<common::TextureCube>(texture_info["pth"].get<std::string>());
            //     textures_cube.insert(kv_texcube(texture_info["name"].get<std::string>(), tex));
            // }
            for (auto &val_info : floatvals)
            {
                std::string name = val_info["name"].get<std::string>();
                float val = val_info["val"].get<float>();
                int idx = glGetUniformLocation(shader->shader, name.c_str());
                float_vals.insert(kv_float(name, val));
                idxs.insert(kv_int(name, idx));
                glUniform1f(idx, val);
            }
            for (auto &val_info : intvals)
            {
                std::string name = val_info["name"].get<std::string>();
                int val = val_info["val"].get<int>();
                int idx = glGetUniformLocation(shader->shader, name.c_str());
                int_vals.insert(kv_int(name, val));
                idxs.insert(kv_int(name, idx));
                glUniform1i(idx, val);
            }
        }

        virtual void PrepareForDraw()
        {
            glUseProgram(shader->shader);
            for (auto &tex : textures_2d)
            {
                glActiveTexture(GL_TEXTURE0 + idxs[tex.first]);
                glBindTexture(GL_TEXTURE_2D, tex.second->texture);
            }
        }

    private:
        std::map<std::string, int> int_vals;
        std::map<std::string, float> float_vals;
        std::map<std::string, std::shared_ptr<common::Texture2D>> textures_2d;
        std::map<std::string, std::shared_ptr<common::TextureCube>> textures_cube;

        std::map<std::string, int> idxs;
    };
}
#endif