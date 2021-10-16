#ifndef GAME_OBJ_H
#define GAME_OBJ_H

#include "common.h"
#include "../render/renderer.h"

namespace common
{
    class GameObject
    {
    public:
        GameObject() {}

        GameObject(std::shared_ptr<renderer::Renderer> rd,
                   std::shared_ptr<Material> material,
                   std::shared_ptr<ModelMesh> mesh) : rd(rd),
                                                      material(material),
                                                      mesh(mesh) {}

        virtual void OnStart()
        {
            render_id = rd->GetRenderID(renderer::OPAQUE);
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

    private:
        std::shared_ptr<renderer::Renderer> rd;
        std::shared_ptr<Material> material;
        std::shared_ptr<ModelMesh> mesh;
        int render_id;
    };
} // namespace common

#endif