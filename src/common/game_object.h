#ifndef GAME_OBJ_H
#define GAME_OBJ_H

#include "common.h"
#include "../render/renderer.h"
#include <memory>

namespace common
{
    struct TransformParameter
    {
        glm::mat4 model;
        glm::mat4 translation;
        glm::mat4 rotation;
        glm::vec3 pos;

        TransformParameter() {}
        TransformParameter(glm::vec3 pos, glm::vec3 dir) : pos(pos)
        {
            rotation = glm::rotate(glm::mat4(1.0f), dir.x, glm::vec3(1.0f, 0.0f, 0.0f));
            rotation = glm::rotate(rotation, dir.y, glm::vec3(0.0f, 1.0f, 0.0f));
            rotation = glm::rotate(rotation, dir.z, glm::vec3(0.0f, 0.0f, 1.0f));
            translation = glm::translate(glm::mat4(1.0f), pos);
            model = translation * rotation;
        }
    };

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

        virtual void OnTransformed(TransformParameter &param) {}
        virtual void OnStart() {}
        virtual void Dispose() {}

    protected:
        std::shared_ptr<GameObject> object;
    };

    class GameObject
    {
    public:
        GameObject() {}

        GameObject(std::shared_ptr<TransformParameter> tparam) : tParam(tparam) {}

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

        std::shared_ptr<TransformParameter> GetTransformInfo()
        {
            return tParam;
        }

        void RotateGlobal(float det, glm::vec3 &axis)
        {
            glm::vec3 gaxis = glm::transpose(tParam->rotation) * glm::vec4(axis, 0.0f);
            tParam->rotation = glm::rotate(tParam->rotation, det, glm::normalize(gaxis));
            UpdateTransform();
        }

        void RotateLocal(float det, glm::vec3 &axis)
        {
            tParam->rotation = glm::rotate(tParam->rotation, det, glm::normalize(axis));
            UpdateTransform();
        }

        void TranslateLocal(glm::vec3 det)
        {
            glm::vec3 gdet = tParam->rotation * glm::vec4(det, 0.0f);
            tParam->translation = glm::translate(tParam->translation, gdet);
            tParam->pos = tParam->translation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            UpdateTransform();
        }

        void TranslateGlobal(glm::vec3 det)
        {
            tParam->translation = glm::translate(tParam->translation, det);
            tParam->pos = tParam->translation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            UpdateTransform();
        }

    protected:
        std::vector<std::shared_ptr<Component>> components;
        std::shared_ptr<TransformParameter> tParam;

    private:
        void UpdateTransform()
        {
            tParam->model = tParam->translation * tParam->rotation;
            for (auto &component : components)
            {
                component->OnTransformed(*tParam);
            }
        }
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
                         std::shared_ptr<common::ModelMesh> mesh,
                         std::shared_ptr<common::RenderArguments> args) : rd(rd),
                                                                          material(material),
                                                                          mesh(mesh), args(args), Component(object) {}
        virtual void OnTransformed(common::TransformParameter &param)
        {
            args->model = param.model;
        }

        virtual void OnStart()
        {
            args->model = object->GetTransformInfo()->model;
            render_id = rd->GetRenderID(renderer::OPAQUE);
            renderer::RenderQueueItem item(material, mesh, args, mesh->id_count);
            rd->InsertToQueue(render_id, item, renderer::OPAQUE);
        }
        virtual void Dispose()
        {
            rd->RemoveFromQueue(render_id, renderer::OPAQUE);
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
        std::shared_ptr<common::RenderArguments> args;
        int render_id;
    };

    class Camera : public common::EventListener, public common::Component
    {
    public:
        glm::mat4 view;
        glm::mat4 projection;

        Camera() {}
        Camera(
            std::shared_ptr<common::GameObject> object,
            std::shared_ptr<renderer::Renderer> rd,
            glm::mat4 projection) : rd(rd), projection(projection), Component(object)
        {
            auto tparam = object->GetTransformInfo();
            glm::vec3 gfront = tparam->rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            glm::vec3 gup = tparam->rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            view = glm::lookAt(tparam->pos, tparam->pos + gfront, gup);
            rd->UpdateVP(view, projection, tparam->pos);
            common::EventTransmitter::GetInstance()->SubscribeEvent(
                common::EventType::EVENT_WINDOW_RESIZE,
                std::static_pointer_cast<common::EventListener>(std::shared_ptr<Camera>(this)));
        }

        virtual void OnWindowResize(std::shared_ptr<common::ED_WindowResize> desc)
        {
            projection = glm::perspective(glm::radians(45.0f), desc->width * 1.0f / desc->height, 0.1f, 100.0f);
            rd->UpdateVP(view, projection, object->GetTransformInfo()->pos);
        }

        virtual void OnTransformed(common::TransformParameter &param)
        {
            glm::vec3 gfront = param.rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            glm::vec3 gup = param.rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            view = glm::lookAt(param.pos, param.pos + gfront, gup);
            rd->UpdateVP(view, projection, object->GetTransformInfo()->pos);
        }

    private:
        std::shared_ptr<renderer::Renderer> rd;
    };

    class Light : public common::Component
    {
    public:
        Light() {}
        Light(
            std::shared_ptr<common::GameObject> object,
            std::shared_ptr<renderer::LightParameters> light_param,
            std::shared_ptr<renderer::Renderer> rd)
            : light_param(light_param), rd(rd), Component(object)
        {
            auto &tparam = object->GetTransformInfo();
            auto &lparam = light_param->inner_params;
            lparam.position = glm::vec4(tparam->pos, lparam.position.w);
            glm::vec3 dir = tparam->rotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            lparam.direction = glm::vec4(dir, lparam.direction.w);
        }

        virtual void OnStart()
        {
            lid = rd->InsertLight(light_param);
        }

        virtual void Dispose()
        {
            rd->RemoveLight(lid);
        }

        virtual void OnTransformed(common::TransformParameter &param)
        {
            auto &lparam = light_param->inner_params;
            lparam.position = glm::vec4(param.pos, lparam.position.w);
            glm::vec3 dir = param.rotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            lparam.direction = glm::vec4(dir, lparam.direction.w);
            rd->UpdateLight(lid);
        }

    private:
        renderer::light_id lid;
        std::shared_ptr<renderer::LightParameters> light_param;
        std::shared_ptr<renderer::Renderer> rd;
    };
}
#endif