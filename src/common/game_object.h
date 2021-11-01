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
        glm::vec3 pos;
        glm::vec3 front;
        glm::vec3 up;
        float yaw;
        float pitch;

        TransformParameter() {}
        TransformParameter(glm::mat4 model, glm::vec3 pos) : model(model), pos(pos)
        {
            up = glm::vec3(0.0f, 1.0f, 0.0f);
            yaw = 270;
            pitch = 0;
            CalcFront();
        }

        void CalcFront()
        {
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            front = glm::normalize(front);
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

        GameObject(TransformParameter tparam) : tParam(tparam) {}

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

        TransformParameter GetTransformInfo()
        {
            return tParam;
        }

        void Rotate(float detYaw, float detPitch)
        {
            //TODO model_matrix
            tParam.yaw += detYaw;
            tParam.pitch += detPitch;
            if (tParam.pitch > 89.0f)
                tParam.pitch = 89.0f;
            if (tParam.pitch < -89.0f)
                tParam.pitch = -89.0f;
            tParam.CalcFront();
            for (auto &component : components)
            {
                component->OnTransformed(tParam);
            }
        }

        void Transform(glm::vec3 det)
        {
            //TODO model_matrix
            //TODO
            tParam.pos += det.x * glm::normalize(glm::cross(tParam.front, tParam.up));
            tParam.pos += det.y * glm::normalize(tParam.front);
            for (auto &component : components)
            {
                component->OnTransformed(tParam);
            }
        }

    protected:
        std::vector<std::shared_ptr<Component>> components;
        TransformParameter tParam;
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
            render_id = rd->GetRenderID(renderer::OPAQUE);
            renderer::RenderQueueItem item(material, mesh, args, mesh->id_count);
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
            view = glm::lookAt(tparam.pos, tparam.pos + tparam.front, tparam.up);
            rd->UpdateVP(view, projection, tparam.pos);
            common::EventTransmitter::GetInstance()->SubscribeEvent(
                common::EventType::EVENT_WINDOW_RESIZE,
                std::static_pointer_cast<common::EventListener>(std::shared_ptr<Camera>(this)));
        }

        virtual void OnWindowResize(std::shared_ptr<common::ED_WindowResize> desc)
        {
            projection = glm::perspective(glm::radians(45.0f), desc->width * 1.0f / desc->height, 0.1f, 100.0f);
            rd->UpdateVP(view, projection, object->GetTransformInfo().pos);
        }

        virtual void OnTransformed(common::TransformParameter &param)
        {
            view = glm::lookAt(param.pos, param.pos + param.front, param.up);
            rd->UpdateVP(view, projection, object->GetTransformInfo().pos);
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
            float intensity,
            glm::vec3 color,
            bool cast_shadow) : intensity(intensity), color(color), cast_shadow(cast_shadow), Component(object) {}

    private:
        float intensity;
        glm::vec3 color;
        bool cast_shadow;
    };

    class SpotLight : public Light
    {
        //position: vec3(in transform) | intensity: float | color: vec3 | range: float | spot angle: float | direction: vec3(in transform)
    public:
        SpotLight() {}
        SpotLight(std::shared_ptr<common::GameObject> object,
                  float intensity,
                  glm::vec3 color,
                  float range,
                  float spot_angle,
                  bool cast_shadow) : range(range), spot_angle(spot_angle), Light(object, intensity, color, cast_shadow) {}

    private:
        float range;
        float spot_angle;
    };

    class PointLight : public Light
    {
        //position(in transform) | intensity | color | range |
    public:
        PointLight() {}
        PointLight(std::shared_ptr<common::GameObject> object,
                   float intensity,
                   glm::vec3 color,
                   float range,
                   bool cast_shadow) : range(range), Light(object, intensity, color, cast_shadow) {}

    private:
        float range;
    };

    class DirectionalLight : public Light
    {
        //position(in transform) | intensity | color | direction(in transform)
    public:
        DirectionalLight() {}
        DirectionalLight(std::shared_ptr<common::GameObject> object,
                         float intensity,
                         glm::vec3 color,
                         bool cast_shadow) : Light(object, intensity, color, cast_shadow) {}
    };
}
#endif