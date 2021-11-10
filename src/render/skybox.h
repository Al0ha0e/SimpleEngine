#ifndef SKYBOX_H
#define SKYBOX_H

#include "../common/common.h"

namespace builtin_materials
{
    struct SkyboxMaterial : public common::Material
    {
        SkyboxMaterial() {}
        SkyboxMaterial(std::shared_ptr<common::ShaderProgram> shader,
                       std::shared_ptr<common::TextureCube> box) : skybox(box), Material(shader, 0)
        {
            glUseProgram(shader->shader);
            glUniform1i(glGetUniformLocation(shader->shader, "skybox"), 0);
        }

        virtual void PrepareForDraw()
        {
            glUseProgram(shader->shader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->texture);
        };

        virtual void Dispose()
        {
            if (shader != nullptr)
            {
                shader->Dispose();
            }
            //TODO texture dispose
        }
        std::shared_ptr<common::TextureCube> skybox;
    };
}

namespace renderer
{
    class SkyBox
    {
    public:
        SkyBox() {}
        SkyBox(std::shared_ptr<builtin_materials::SkyboxMaterial> material);

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
    };
}

#endif