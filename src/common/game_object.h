#ifndef GAME_OBJ_H
#define GAME_OBJ_H

#include "common.h"
#include "../render/renderer.h"
#include "materials.h"
#include <memory>

namespace common
{
    struct TransformParameter
    {
        glm::mat4 model;
        glm::mat4 translation;
        glm::mat4 rotation;
        glm::vec3 pos;
        glm::vec3 dir;

        TransformParameter() {}
        TransformParameter(glm::vec3 pos, glm::vec3 dir) : pos(pos), dir(dir)
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
        ~Component() {}
        Component(std::shared_ptr<GameObject> object) : object(object) {}
        std::weak_ptr<GameObject> GetObject()
        {
            return object;
        }

        virtual void OnTransformed(TransformParameter &param) {}
        virtual void OnStart() {}
        virtual void Dispose() {}
        virtual void UnserializeJSON(nlohmann::json &j, std::shared_ptr<GameObject> obj, resources::ResourceManager *manager) {}
        virtual std::string SerializeJSON() { return ""; }

    protected:
        std::weak_ptr<GameObject> object;
    };

    std::shared_ptr<Component> UnserializeComponent(
        std::string tp,
        nlohmann::json &j,
        std::shared_ptr<GameObject> obj,
        resources::ResourceManager *manager);

    std::string SerializeComponent(Component &component);

    class GameObject : public resources::SerializableObject
    {
    public:
        GameObject() {}
        ~GameObject() {}
        GameObject(std::shared_ptr<renderer::Renderer> rd) : rd(rd) {}
        GameObject(std::shared_ptr<TransformParameter> tparam,
                   std::shared_ptr<renderer::Renderer> rd) : tParam(tparam), rd(rd) {}

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

        virtual void UnserializeJSON(nlohmann::json &j, resources::ResourceManager *manager)
        {
            auto pos = j["pos"];
            auto dir = j["dir"];
            tParam = std::make_shared<TransformParameter>(
                glm::vec3(pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>()),
                glm::vec3(dir[0].get<float>(), dir[1].get<float>(), dir[2].get<float>()));
            auto comp = j["components"];
            for (auto &component : comp)
            {
                AddComponent(UnserializeComponent(component["tp"].get<std::string>(),
                                                  component["content"],
                                                  self,
                                                  manager));
            }
        }

        virtual std::string SerializeJSON()
        {
            std::string ret = "{\n";
            ret += "\"pos\": [";
            for (int i = 0; i < 3; i++)
                ret += std::to_string(tParam->pos[i]) + ",";
            ret[ret.length() - 1] = ']';
            ret += ",\n\"dir\": [";
            for (int i = 0; i < 3; i++)
                ret += std::to_string(tParam->dir[i]) + ",";
            ret[ret.length() - 1] = ']';
            ret += ",\n\"components\": [";
            for (auto &component : components)
                ret += "\n" + SerializeComponent(*component) + ",";
            ret[ret.length() - 1] = ']';
            ret += "\n}";
            return ret;
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
        std::shared_ptr<renderer::Renderer> rd;
        std::shared_ptr<GameObject> self; //TODO Manually release it

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
                         std::string material_pth,
                         std::string mesh_pth,
                         resources::ResourceManager *manager) : Component(object)
        {
            init(material_pth, mesh_pth, manager);
        }
        void init(std::string material_pth, std::string mesh_pth, resources::ResourceManager *manager)
        {
            this->material_pth = material_pth;
            this->mesh_pth = mesh_pth;
            material = manager->LoadMeta<builtin_materials::CustomMaterial>(material_pth);
            mesh = manager->Load<common::ModelMesh>(mesh_pth);
            args = std::make_shared<common::RenderArguments>();
            if (!material->material_id)
            {
                auto locked_object = object.lock();
                unsigned int id = locked_object->rd->GetMaterialID(renderer::RenderMode(material->render_mode));
                material->material_id = id;
                locked_object->rd->RegisterMaterial(id, material, renderer::RenderMode(material->render_mode));
            }
        }

        virtual void OnTransformed(common::TransformParameter &param)
        {
            args->model = param.model;
        }

        virtual void OnStart()
        {
            auto locked_object = object.lock();
            args->model = locked_object->GetTransformInfo()->model;
            render_id = locked_object->rd->GetRenderID(renderer::OPAQUE);
            renderer::RenderQueueItem item(material, mesh, args, mesh->id_count);
            locked_object->rd->InsertToQueue(render_id, item, renderer::OPAQUE);
        }

        virtual void UnserializeJSON(nlohmann::json &j, std::shared_ptr<common::GameObject> obj, resources::ResourceManager *manager)
        {
            this->object = obj;
            init(j["material"].get<std::string>(), j["mesh"].get<std::string>(), manager);
        }

        virtual std::string SerializeJSON()
        {
            std::string ret = "{\n";
            ret += "\"material\": \"" + material_pth + "\",\n";
            ret += "\"mesh\": \"" + mesh_pth + "\"";
            ret += "\n}";
            return ret;
        }

    private:
        std::shared_ptr<common::Material> material;
        std::shared_ptr<common::ModelMesh> mesh;
        std::shared_ptr<common::RenderArguments> args;
        std::string material_pth;
        std::string mesh_pth;
        int render_id;
    };

    class Camera : public common::EventListener, public common::Component
    {
    public:
        glm::mat4 view;
        glm::mat4 projection;

        Camera() {}
        ~Camera() {}
        Camera(
            std::shared_ptr<common::GameObject> object,
            float fov,
            float aspect,
            float near,
            float far) : Component(object)
        {
            init(fov, aspect, near, far);
        }

        void init(float fov, float aspect, float near, float far)
        {
            this->fov = fov;
            this->aspect = aspect;
            this->near = near;
            this->far = far;
            this->projection = glm::perspective(fov, aspect, near, far);
            auto locked_object = object.lock();
            auto tparam = locked_object->GetTransformInfo();
            glm::vec3 gfront = tparam->rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            glm::vec3 gup = tparam->rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            view = glm::lookAt(tparam->pos, tparam->pos + gfront, gup);
            locked_object->rd->UpdateVP(view, projection, tparam->pos);
            common::EventTransmitter::GetInstance()->SubscribeEvent(
                common::EventType::EVENT_WINDOW_RESIZE,
                std::static_pointer_cast<common::EventListener>(std::shared_ptr<Camera>(this)));
        }

        virtual void OnWindowResize(std::shared_ptr<common::ED_WindowResize> desc)
        {
            auto locked_object = object.lock();
            projection = glm::perspective(glm::radians(45.0f), desc->width * 1.0f / desc->height, 0.1f, 100.0f);
            locked_object->rd->UpdateVP(view, projection, locked_object->GetTransformInfo()->pos);
        }

        virtual void OnTransformed(common::TransformParameter &param)
        {
            auto locked_object = object.lock();
            glm::vec3 gfront = param.rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            glm::vec3 gup = param.rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            view = glm::lookAt(param.pos, param.pos + gfront, gup);
            locked_object->rd->UpdateVP(view, projection, locked_object->GetTransformInfo()->pos);
        }

        virtual void UnserializeJSON(nlohmann::json &j, std::shared_ptr<common::GameObject> obj, resources::ResourceManager *manager)
        {
            this->object = obj;
            init(j["fov"].get<float>(),
                 j["aspect"].get<float>(),
                 j["near"].get<float>(),
                 j["far"].get<float>());
        }

        virtual std::string SerializeJSON()
        {
            std::string ret = "{\n";
            ret += "\"fov\": " + std::to_string(fov) + ",\n";
            ret += "\"aspect\": " + std::to_string(aspect) + ",\n";
            ret += "\"near\": " + std::to_string(near) + ",\n";
            ret += "\"far\": " + std::to_string(far);
            ret += "\n}";
            return ret;
        }

    private:
        float fov;
        float aspect;
        float near;
        float far;
    };

    class Light : public common::Component
    {
    public:
        Light() {}
        Light(
            std::shared_ptr<common::GameObject> object,
            std::shared_ptr<renderer::LightParameters> light_param)
            : light_param(light_param), Component(object)
        {
            auto &tparam = object->GetTransformInfo();
            auto &lparam = light_param->inner_params;
            lparam.position = glm::vec4(tparam->pos, lparam.position.w);
            glm::vec3 dir = tparam->rotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            lparam.direction = glm::vec4(dir, lparam.direction.w);
        }

        virtual void OnStart()
        {
            lid = object.lock()->rd->InsertLight(light_param);
        }

        virtual void OnTransformed(common::TransformParameter &param)
        {
            auto &lparam = light_param->inner_params;
            lparam.position = glm::vec4(param.pos, lparam.position.w);
            glm::vec3 dir = param.rotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            lparam.direction = glm::vec4(dir, lparam.direction.w);
            object.lock()->rd->UpdateLight(lid);
        }

        virtual void UnserializeJSON(nlohmann::json &j, std::shared_ptr<common::GameObject> obj, resources::ResourceManager *manager)
        {
            this->object = obj;
            auto &tparam = obj->GetTransformInfo();
            glm::vec3 dir = tparam->rotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            auto color = j["color"];
            light_param = std::make_shared<renderer::LightParameters>(
                renderer::LightType(j["tp"].get<unsigned int>()),
                j["cast_shadow"].get<bool>(),
                renderer::InnerLightParameters(
                    glm::vec4(tparam->pos, j["intensity"].get<float>()),
                    glm::vec4(color[0].get<float>(), color[1].get<float>(), color[2].get<float>(), j["range"].get<float>()),
                    glm::vec4(dir, j["spot_angle"].get<float>())));
        }

        virtual std::string SerializeJSON()
        {
            std::string ret = "{\n";
            auto &param = *light_param;
            ret += "\"tp\": " + std::to_string(param.tp) + ",\n";
            ret += "\"cast_shadow\": " + std::string(param.cast_shadow ? "true" : "false") + ",\n";
            ret += "\"color\": [";
            for (int i = 0; i < 3; i++)
                ret += std::to_string(param.inner_params.color[i]) + ",";
            ret[ret.length() - 1] = ']';
            ret += ",\n\"intensity\": " + std::to_string(param.inner_params.position[3]) + ",\n";
            ret += "\"range\": " + std::to_string(param.inner_params.color[3]) + ",\n";
            ret += "\"spot_angle\": " + std::to_string(param.inner_params.direction[3]);
            ret += "\n}";
            return ret;
        }

    private:
        renderer::light_id lid;
        std::shared_ptr<renderer::LightParameters> light_param;
    };
}
#endif