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
        unsigned int vertex_cnt;

        virtual void Draw(glm::mat4 view, glm::mat4 projection)
        {
            // material->UpdateV(view);
            // material->UpdateP(projection);
            material->PrepareForDraw();
            mesh->PrepareForDraw();
            glDrawElements(GL_TRIANGLES, vertex_cnt, GL_UNSIGNED_INT, 0);
        }

        RenderQueueItem() {}
        RenderQueueItem(std::shared_ptr<common::Material> material,
                        std::shared_ptr<common::ModelMesh> mesh,
                        unsigned int vertex_cnt) : material(material), mesh(mesh), vertex_cnt(vertex_cnt) {}
    };

    typedef unsigned long long render_id;

    struct RenderQueue
    {
        std::map<render_id, RenderQueueItem> queue;
        render_id maxid;

        render_id GetRenderID() { return ++maxid; }

        void InsertItem(render_id id, RenderQueueItem item)
        {
            queue[id] = item;
        }

        void RemoveItem(render_id id)
        {
            if (queue.find(id) == queue.end())
                return;
            queue.erase(queue.find(id));
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
        void InsertToQueue(render_id id, RenderQueueItem item, RenderMode mode);
        void RemoveFromQueue(render_id id, RenderMode mode);

        light_id GetLightID(bool cast_shadow);
        void InsertToLightQueue(light_id id, LightParameters item, bool cast_shadow);
        void RemoveFromLightQueue(light_id id, bool cast_shadow);

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
        LightQueue cast_queue;
        LightQueue non_cast_queue;
        glm::mat4 view;
        glm::mat4 projection;
        unsigned int ubo_VP;
    };
}

#endif