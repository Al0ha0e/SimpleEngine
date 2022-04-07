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
        auto &layers = RenderLayerManager::GetInstance()->layers;
        for (auto &layer : layers)
            render(layer.GetQueue(OPAQUE), false);
    }

    void Renderer::render(std::shared_ptr<render_queue_node> &now, bool include)
    {
        for (auto &obj : now->content.objects)
        {
            if (include || cam_param.Test(obj->mesh->box) != CameraParameters::FRUSTUM_SEPARATE)
            {
                obj->material->PrepareForDraw();
                obj->Draw(obj->material->shader->shader);
            }
        }

        if (now->subnodes[0] != nullptr)
        {
            for (auto &subnode : now->subnodes)
            {
                CameraParameters::frustum_relation rel = cam_param.Test(subnode->box);
                if (rel == CameraParameters::FRUSTUM_INCLUDE)
                {
                    render(subnode, true);
                }
                else if (rel == CameraParameters::FRUSTUM_INTERSECT)
                {
                    render(subnode, false);
                }
            }
        }
    }
}