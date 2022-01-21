#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "../common/common.h"
#include "skybox.h"
#include "render_queue.h"
#include "../events/event.h"
#include "light.h"

#include <vector>
#include <list>
#include <map>
#include <set>

namespace renderer
{
    enum RenderMode
    {
        OPAQUE,
        OPAQUE_SHADOW,
        TRANSPARENT,
        TRANSPARENT_SHADOW
    };

    typedef unsigned long long render_id;

    struct RenderQueue
    {
        std::map<render_id, RenderQueueItem> queue;
        std::map<render_id, std::shared_ptr<common::Material>> materials;
        std::map<render_id, std::set<render_id>> mat2obj;
        render_id maxid;
        render_id max_material_id;

        render_id GetRenderID() { return ++maxid; }
        render_id GetMaterialID() { return ++max_material_id; }

        void RegisterMaterial(render_id id, std::shared_ptr<common::Material> material)
        {
            materials.insert(std::pair<render_id, std::shared_ptr<common::Material>>(id, material));
            mat2obj.insert(std::pair<render_id, std::set<render_id>>(id, std::set<render_id>()));
        }

        void InsertItem(render_id id, RenderQueueItem item)
        {
            queue[id] = item;
            mat2obj[item.material->material_id].insert(id);
        }

        void RemoveItem(render_id id)
        {
            auto it = queue.find(id);
            if (it == queue.end())
                return;
            mat2obj[it->second.material->material_id].erase(id);
            queue.erase(it);
        }

        void Draw()
        {
            for (auto &material : materials)
            {
                auto &objs = mat2obj[material.first];
                if (objs.size())
                {
                    material.second->PrepareForDraw();
                    for (auto &obj_id : objs)
                        queue[obj_id].Draw(material.second->shader->shader);
                }
            }
        }
    };

    class Renderer
    {
    public:
        Renderer() {}
        Renderer(glm::vec4 ambient, std::shared_ptr<SkyBox> skybox) : ambient(ambient), skybox(skybox)
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glGenBuffers(1, &ubo_VP);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_VP);
            glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_VP);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glGenBuffers(1, &ubo_GI);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_GI);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_GI);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), glm::value_ptr(ambient));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        void Render();
        render_id GetRenderID(RenderMode mode);
        render_id GetMaterialID(RenderMode mode);
        void RegisterMaterial(render_id id, std::shared_ptr<common::Material> material, RenderMode mode);
        void InsertToQueue(render_id id, RenderQueueItem item, RenderMode mode);
        void RemoveFromQueue(render_id id, RenderMode mode);
        light_id InsertLight(std::shared_ptr<LightParameters> light)
        {
            return light_manager.InsertItem(light);
        }
        void UpdateLight(light_id id)
        {
            light_manager.UpdateItem(id);
        }
        void RemoveLight(light_id id)
        {
            light_manager.RemoveItem(id);
        }

        void UpdateVP(glm::mat4 view,
                      glm::mat4 projection,
                      glm::vec3 viewPos)
        {
            this->view = view;
            this->projection = projection;
            this->viewPos = glm::vec4(viewPos, 0.0f);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_VP);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::vec4), glm::value_ptr(this->viewPos));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

    private:
        RenderQueue opaque_queue;
        RenderQueue transparent_queue;
        RenderQueue opaque_shadow_queue;
        RenderQueue transparent_shadow_queue;
        LightManager light_manager;
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 viewPos;
        glm::vec4 ambient;
        unsigned int ubo_VP;
        unsigned int ubo_GI;

        std::shared_ptr<SkyBox> skybox;
    };
}

#endif