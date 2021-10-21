#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "../common/common.h"
#include "../events/event.h"

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
            material->UpdateV(view);
            material->UpdateP(projection);
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

    typedef unsigned long long light_id;

    enum LightType
    {
        POINT_LIGHT,
        SPOT_LIGHT,
        DIRECTIONAL_LIGHT
    };

    struct LightParameters
    {
        LightType tp;
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 direction;
        glm::vec3 misc; //x: intensity y: range z: spot angle
    };

    //TODO Use generic to reduce it
    struct LightQueue
    {
        std::map<light_id, LightParameters> queue;
        light_id maxid;

        light_id GetLightID() { return ++maxid; }

        void InsertItem(render_id id, LightParameters item)
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
    };
}

#endif