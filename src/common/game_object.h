#ifndef GAME_OBJ_H
#define GAME_OBJ_H

#include "common.h"
#include "../render/renderer.h"
#include <memory>

namespace common
{
    class GameObject
    {
    public:
        GameObject() {}

        GameObject(std::shared_ptr<renderer::Renderer> rd,
                   std::shared_ptr<Material> material,
                   std::shared_ptr<ModelMesh> mesh,
                   glm::mat4 model_matrix) : rd(rd),
                                             material(material),
                                             mesh(mesh),
                                             model_matrix(model_matrix) {}

        virtual void OnStart()
        {
            render_id = rd->GetRenderID(renderer::OPAQUE);
            material->UpdateM(model_matrix);
            renderer::RenderQueueItem item(material, mesh, mesh->id_count);
            rd->InsertToQueue(render_id, item, renderer::OPAQUE);
        }
        virtual void BeforeRender() {}
        virtual void AfterRender() {}

        virtual void Dispose()
        {
            if (mesh != nullptr)
                mesh->Dispose();
            if (material != nullptr)
                material->Dispose();
        }

    protected:
        std::shared_ptr<renderer::Renderer> rd;
        std::shared_ptr<Material> material;
        std::shared_ptr<ModelMesh> mesh;
        glm::mat4 model_matrix;
        int render_id;
    };

} // namespace common

#endif