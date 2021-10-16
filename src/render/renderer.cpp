#include "renderer.h"
namespace renderer
{
    void Renderer::Render()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto &item : opaque_queue.queue)
        {
            item.second.Draw(main_camera->view, main_camera->projection);
        }

        glBindVertexArray(0);
    }

    render_id Renderer::GetRenderID(RenderMode mode)
    {
        switch (mode)
        {
        case OPAQUE:
            return opaque_queue.GetRenderID();
        case OPAQUE_SHADOW:
            return opaque_shadow_queue.GetRenderID();
        case TRANSPARENT:
            return transparent_queue.GetRenderID();
        case TRANSPARENT_SHADOW:
            return transparent_shadow_queue.GetRenderID();
        default:
            return 0;
        }
    }

    void Renderer::InsertToQueue(render_id id, RenderQueueItem item, RenderMode mode)
    {
        switch (mode)
        {
        case OPAQUE:
            opaque_queue.InsertItem(id, item);
            break;
        case OPAQUE_SHADOW:
            opaque_shadow_queue.InsertItem(id, item);
            break;
        case TRANSPARENT:
            transparent_queue.InsertItem(id, item);
            break;
        case TRANSPARENT_SHADOW:
            transparent_shadow_queue.InsertItem(id, item);
            break;
        default:
            break;
        }
    }

    void Renderer::RemoveFromQueue(render_id id, RenderMode mode)
    {
        switch (mode)
        {
        case OPAQUE:
            opaque_queue.RemoveItem(id);
            break;
        case OPAQUE_SHADOW:
            opaque_shadow_queue.RemoveItem(id);
            break;
        case TRANSPARENT:
            transparent_queue.RemoveItem(id);
            break;
        case TRANSPARENT_SHADOW:
            transparent_shadow_queue.RemoveItem(id);
            break;
        default:
            break;
        }
    }
}