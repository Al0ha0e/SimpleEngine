#ifndef GAME_OBJ_H
#define GAME_OBJ_H

#include "common.h"
#include "../render/renderer.h"
#include <memory>

namespace common
{
    class GameObject;
    class Component
    {
    public:
        Component() {}
        Component(std::shared_ptr<GameObject> object) : object(object) {}
        std::shared_ptr<GameObject> GetObject()
        {
            return object;
        }

        virtual void OnTransformed() {}
        virtual void OnStart() {}
        virtual void Dispose() {}

    protected:
        std::shared_ptr<GameObject> object;
    };

    class GameObject
    {
    public:
        GameObject() {}

        GameObject(
            glm::mat4 model_matrix) : model_matrix(model_matrix) {}

        void AddComponent(std::shared_ptr<Component> component)
        {
            components.push_back(component);
        }

        virtual void OnStart()
        {
            for (auto &component : components)
            {
                component->OnStart();
            }
        }
        virtual void BeforeRender() {}
        virtual void AfterRender() {}

        virtual void Dispose()
        {
            for (auto &component : components)
            {
                component->Dispose();
            }
        }

        glm::mat4 GetTransformInfo()
        {
            return model_matrix;
        }

    protected:
        std::vector<std::shared_ptr<Component>> components;

        glm::mat4 model_matrix;
    };

} // namespace common

namespace builtin_components
{
    class RenderableObject : public common::Component
    {
    public:
        RenderableObject() {}
        RenderableObject(std::shared_ptr<common::GameObject> object,
                         std::shared_ptr<renderer::Renderer> rd,
                         std::shared_ptr<common::Material> material,
                         std::shared_ptr<common::ModelMesh> mesh) : rd(rd),
                                                                    material(material),
                                                                    mesh(mesh), Component(object) {}
        virtual void OnStart()
        {
            render_id = rd->GetRenderID(renderer::OPAQUE);
            material->UpdateM(object->GetTransformInfo());
            renderer::RenderQueueItem item(material, mesh, mesh->id_count);
            rd->InsertToQueue(render_id, item, renderer::OPAQUE);
        }
        virtual void Dispose()
        {
            if (mesh != nullptr)
                mesh->Dispose();
            if (material != nullptr)
                material->Dispose();
            object = nullptr;
        }

    private:
        std::shared_ptr<renderer::Renderer> rd;
        std::shared_ptr<common::Material> material;
        std::shared_ptr<common::ModelMesh> mesh;
        int render_id;
    };
}
#endif