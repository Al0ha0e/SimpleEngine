#ifndef GAME_OBJ_H
#define GAME_OBJ_H

#include "common.h"
#include "../render/renderer.h"
#include "materials.h"
#include <memory>
#include <algorithm>

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
        virtual void UnserializeJSON(nlohmann::json &j, std::shared_ptr<GameObject> obj) {}
        virtual std::string SerializeJSON() { return ""; }

    protected:
        std::weak_ptr<GameObject> object;
    };

    std::shared_ptr<Component> UnserializeComponent(
        std::string tp,
        nlohmann::json &j,
        std::shared_ptr<GameObject> obj);

    std::string SerializeComponent(Component &component);

    class GameObject : public resources::SerializableObject
    {
    public:
        GameObject()
        {
            id = ++object_cnt;
        }
        ~GameObject() {}
        GameObject(std::shared_ptr<renderer::Renderer> rd) : rd(rd)
        {
            id = ++object_cnt;
        }
        GameObject(std::shared_ptr<TransformParameter> tparam,
                   std::shared_ptr<renderer::Renderer> rd) : tParam(tparam), rd(rd)
        {
            id = ++object_cnt;
        }

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

        virtual void UnserializeJSON(nlohmann::json &j)
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
                                                  self));
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
        std::shared_ptr<GameObject> self; // TODO Manually release it
        unsigned int id;

    protected:
        std::vector<std::shared_ptr<Component>> components;
        std::shared_ptr<TransformParameter> tParam;

    private:
        static unsigned int object_cnt;
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
                         std::string mesh_pth) : Component(object)
        {
            init(material_pth, mesh_pth);
        }
        void init(std::string material_pth, std::string mesh_pth)
        {
            this->material_pth = material_pth;
            this->mesh_pth = mesh_pth;
            material = resources::LoadMeta<builtin_materials::CustomMaterial>(material_pth);
            mesh = resources::Load<common::ModelMesh>(mesh_pth);
            args = std::make_shared<common::RenderArguments>();
            item = std::make_shared<renderer::RenderQueueItem>(object.lock()->id, material, mesh, args, mesh->id_count);
            // TODO
            rd_idxs.push_back(renderer::RenderLayerIndex(0, true, false, false));
        }

        virtual void OnTransformed(common::TransformParameter &param)
        {
            updateRenderParam(param.model);
        }

        virtual void OnStart()
        {
            auto locked_object = object.lock();
            updateRenderParam(locked_object->GetTransformInfo()->model);

            for (auto &idx : rd_idxs)
            {
                auto &layer = renderer::RenderLayerManager::GetInstance()->layers[idx.layer];
                if (idx.in_opaque)
                    layer.InsertObject(renderer::OPAQUE, item);
                if (idx.in_transparent)
                    layer.InsertObject(renderer::TRANSPARENT, item);
                if (idx.in_shadow)
                    layer.InsertObject(renderer::OPAQUE_SHADOW, item);
            }
        }

        virtual void UnserializeJSON(nlohmann::json &j, std::shared_ptr<common::GameObject> obj)
        {
            this->object = obj;
            init(j["material"].get<std::string>(), j["mesh"].get<std::string>());
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
        std::shared_ptr<renderer::RenderQueueItem> item;
        std::shared_ptr<common::Material> material;
        std::shared_ptr<common::ModelMesh> mesh;
        std::shared_ptr<common::RenderArguments> args;
        std::string material_pth;
        std::string mesh_pth;
        std::vector<renderer::RenderLayerIndex> rd_idxs;

        void updateRenderParam(glm::mat4 &model)
        {
            args->model = model;
            common::BoundingBox &mbox = mesh->box;
            common::BoundingBox &box = args->box;

            glm::vec3 tmin = model * glm::vec4(mbox.min, 1);
            glm::vec3 tmax = model * glm::vec4(mbox.max, 1);
            glm::vec3 nmin(std::min(tmin.x, tmax.x), std::min(tmin.y, tmax.y), std::min(tmin.z, tmax.z));
            glm::vec3 nmax(std::max(tmin.x, tmax.x), std::max(tmin.y, tmax.y), std::max(tmin.z, tmax.z));
            box = common::BoundingBox(nmin, nmax, 1.1f);
        }
    };

    class Camera : public common::EventListener, public common::Component
    {
    public:
        glm::mat4 view;
        bool is_sub;

        Camera() {}
        ~Camera() {}
        Camera(
            std::shared_ptr<common::GameObject> object,
            float fov,
            float aspect,
            float near,
            float far,
            bool is_sub) : is_sub(is_sub), Component(object)
        {
            init(fov, aspect, near, far);
        }

        void init(float fov, float aspect, float near, float far)
        {
            this->fov = fov;
            this->aspect = aspect;
            this->near = near;
            this->far = far;
            auto locked_object = object.lock();
            auto tparam = locked_object->GetTransformInfo();
            glm::vec3 gfront = tparam->rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            glm::vec3 gup = tparam->rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            view = glm::lookAt(tparam->pos, tparam->pos + gfront, gup);
            locked_object->rd->UpdateView(view, tparam->pos, is_sub);
            locked_object->rd->UpdateProjection(fov, aspect, near, far, is_sub);
            common::EventTransmitter::GetInstance()->SubscribeEvent(
                common::EventType::EVENT_WINDOW_RESIZE,
                std::static_pointer_cast<common::EventListener>(std::shared_ptr<Camera>(this)));
        }

        virtual void OnWindowResize(std::shared_ptr<common::ED_WindowResize> desc)
        {
            auto locked_object = object.lock();
            aspect = desc->width * 1.0f / desc->height;
            locked_object->rd->UpdateProjection(fov, aspect, near, far, is_sub);
        }

        virtual void OnTransformed(common::TransformParameter &param)
        {
            auto locked_object = object.lock();
            glm::vec3 gfront = param.rotation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
            glm::vec3 gup = param.rotation * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
            view = glm::lookAt(param.pos, param.pos + gfront, gup);
            locked_object->rd->UpdateView(view, param.pos, is_sub);
        }

        virtual void UnserializeJSON(nlohmann::json &j, std::shared_ptr<common::GameObject> obj)
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
            lid = renderer::LightManager::GetInstance()->InsertItem(light_param);
        }

        virtual void OnTransformed(common::TransformParameter &param)
        {
            auto &lparam = light_param->inner_params;
            lparam.position = glm::vec4(param.pos, lparam.position.w);
            glm::vec3 dir = param.rotation * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
            lparam.direction = glm::vec4(dir, lparam.direction.w);
            renderer::LightManager::GetInstance()->UpdateItem(lid);
        }

        virtual void UnserializeJSON(nlohmann::json &j, std::shared_ptr<common::GameObject> obj)
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