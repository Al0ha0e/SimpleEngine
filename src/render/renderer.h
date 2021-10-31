#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "../common/common.h"
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

    struct RenderQueueItem
    {
        std::shared_ptr<common::Material> material;
        std::shared_ptr<common::ModelMesh> mesh;
        std::shared_ptr<common::RenderArguments> args;
        unsigned int vertex_cnt;

        virtual void Draw(unsigned int shader_id)
        {
            mesh->PrepareForDraw();
            args->PrepareForDraw(shader_id);
            glDrawElements(GL_TRIANGLES, vertex_cnt, GL_UNSIGNED_INT, 0);
        }

        RenderQueueItem() {}
        RenderQueueItem(std::shared_ptr<common::Material> material,
                        std::shared_ptr<common::ModelMesh> mesh,
                        std::shared_ptr<common::RenderArguments> args,
                        unsigned int vertex_cnt) : material(material), mesh(mesh), args(args), vertex_cnt(vertex_cnt) {}
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
        Renderer(glm::mat4 view,
                 glm::mat4 projection) : view(view), projection(projection)
        {
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glGenBuffers(1, &ubo_VP);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_VP);
            glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_VP);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        void Render();
        render_id GetRenderID(RenderMode mode);
        render_id GetMaterialID(RenderMode mode);
        void RegisterMaterial(render_id id, std::shared_ptr<common::Material> material, RenderMode mode);
        void InsertToQueue(render_id id, RenderQueueItem item, RenderMode mode);
        void RemoveFromQueue(render_id id, RenderMode mode);

        void UpdateVP(glm::mat4 view,
                      glm::mat4 projection)
        {
            this->view = view;
            this->projection = projection;
        }

    private:
        RenderQueue opaque_queue;
        RenderQueue transparent_queue;
        RenderQueue opaque_shadow_queue;
        RenderQueue transparent_shadow_queue;
        glm::mat4 view;
        glm::mat4 projection;
        unsigned int ubo_VP;
    };
}

#endif