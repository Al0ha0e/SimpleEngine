#include "renderer.h"
namespace renderer
{
    void Renderer::Render()
    {
        glClearColor(0.0f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthMask(GL_FALSE);
        skybox->Draw();
        glDepthMask(GL_TRUE);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo_GI);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4), glm::value_ptr(ambient));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        opaque_queue.Draw();
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
        default:
            return 0;
        }
    }

    render_id Renderer::GetMaterialID(RenderMode mode)
    {
        switch (mode)
        {
        case OPAQUE:
            return opaque_queue.GetMaterialID();
        case OPAQUE_SHADOW:
            return opaque_shadow_queue.GetMaterialID();
        case TRANSPARENT:
            return transparent_queue.GetMaterialID();
        default:
            return 0;
        }
    }

    void Renderer::RegisterMaterial(render_id id, std::shared_ptr<common::Material> material, RenderMode mode)
    {
        switch (mode)
        {
        case OPAQUE:
            opaque_queue.RegisterMaterial(id, material);
            break;
        case OPAQUE_SHADOW:
            opaque_shadow_queue.RegisterMaterial(id, material);
            break;
        case TRANSPARENT:
            transparent_queue.RegisterMaterial(id, material);
            break;
        default:
            break;
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
        default:
            break;
        }
    }
}