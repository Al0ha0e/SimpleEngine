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

    class Camera : public common::EventListener
    {
    public:
        glm::mat4 view;
        glm::mat4 projection;

        Camera() {}
        Camera(glm::mat4 view,
               glm::mat4 projection) : view(view), projection(projection)
        {
            common::EventTransmitter::GetInstance()->SubmitEvent(
                common::EventType::EVENT_WINDOW_RESIZE,
                std::static_pointer_cast<common::EventListener>(std::shared_ptr<Camera>(this)));
        }
        virtual void OnWindowResize(std::shared_ptr<common::ED_WindowResize> desc)
        {
            projection = glm::perspective(glm::radians(45.0f), desc->width * 1.0f / desc->height, 0.1f, 100.0f);
        }
    };

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
        std::map<unsigned int, RenderQueueItem> queue;
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
        Renderer(std::shared_ptr<Camera> main_camera) : main_camera(main_camera)
        {
            glEnable(GL_DEPTH_TEST);
        }
        void Render();
        render_id GetRenderID(RenderMode mode);
        void InsertToQueue(render_id id, RenderQueueItem item, RenderMode mode);
        void RemoveFromQueue(render_id id, RenderMode mode);

    private:
        RenderQueue opaque_queue;
        RenderQueue transparent_queue;
        RenderQueue opaque_shadow_queue;
        RenderQueue transparent_shadow_queue;
        std::shared_ptr<Camera> main_camera;
    };
}
#endif