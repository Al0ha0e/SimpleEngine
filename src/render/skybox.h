#ifndef SKYBOX_H
#define SKYBOX_H

#include "../common/common.h"
#include "../resource/resource.h"

namespace builtin_materials
{

    struct SkyboxMaterial : public common::Material
    {
        SkyboxMaterial() {}
        SkyboxMaterial(std::shared_ptr<common::ShaderProgram> shader,
                       std::shared_ptr<common::Texture2D> box) : skybox(box), Material(shader, 0)
        {
            glUseProgram(shader->shader);
            glUniform1i(glGetUniformLocation(shader->shader, "skybox"), 0 + common::ENGINE_TEXTURE_CNT);
        }

        virtual void PrepareForDraw()
        {
            glUseProgram(shader->shader);
            glActiveTexture(GL_TEXTURE0 + common::ENGINE_TEXTURE_CNT);
            glBindTexture(GL_TEXTURE_2D, skybox->texture);
        };

        virtual void Dispose()
        {
            if (shader != nullptr)
            {
                shader->Dispose();
            }
            //TODO texture dispose
        }
        std::shared_ptr<common::Texture2D> skybox;
    };

}

namespace renderer
{
    class SkyBox : public resources::SerializableObject
    {
    public:
        SkyBox() {}
        SkyBox(std::shared_ptr<builtin_materials::SkyboxMaterial> material) : material(material) { this->init(); }
        virtual void UnserializeJSON(std::string s, resources::ResourceManager *manager)
        {
            auto j = nlohmann::json::parse(s);
            std::string shaderpth = j["shader"].get<std::string>();
            std::string boxpth = j["skybox"].get<std::string>();
            std::string irrpth = j["irr"].get<std::string>();
            std::string prefpth = j["pref"].get<std::string>();
            auto shader = manager->LoadMeta<common::ShaderProgram>(shaderpth);
            auto box = manager->LoadMeta<common::Texture2D>(boxpth);
            this->irradiance_map = manager->LoadMeta<common::Texture2D>(irrpth);
            this->prefiltered_map = manager->LoadMeta<common::Texture2D>(prefpth);
            this->lut_map = manager->LoadMeta<common::Texture2D>("./src/render/lut.json");
            this->material = std::make_shared<builtin_materials::SkyboxMaterial>(shader, box);

            this->init();
        }
        void Draw()
        {
            material->PrepareForDraw();
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

    private:
        unsigned int vao;
        unsigned int vbo;
        std::shared_ptr<builtin_materials::SkyboxMaterial> material;
        std::shared_ptr<common::Texture2D> irradiance_map;
        std::shared_ptr<common::Texture2D> prefiltered_map;
        std::shared_ptr<common::Texture2D> lut_map;
        void init();
    };
}

#endif