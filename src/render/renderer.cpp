#include "renderer.h"
namespace renderer
{
    void Renderer::Render()
    {
        glClearColor(0.0f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_VP);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(view));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        for (auto &item : opaque_queue.queue)
        {
            item.second.Draw(view, projection);
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

    light_id Renderer::GetLightID(bool cast_shadow)
    {
        if (cast_shadow)
            return cast_queue.GetLightID();
        return non_cast_queue.GetLightID();
    }
    void Renderer::InsertToLightQueue(light_id id, LightParameters item, bool cast_shadow)
    {
        if (cast_shadow)
        {
            cast_queue.InsertItem(id, item);
        }
        else
        {
            non_cast_queue.InsertItem(id, item);
        }
    }
    void Renderer::RemoveFromLightQueue(light_id id, bool cast_shadow)
    {
        if (cast_shadow)
        {
            cast_queue.RemoveItem(id);
        }
        else
        {
            non_cast_queue.RemoveItem(id);
        }
    }

}