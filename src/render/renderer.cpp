#include "renderer.h"
#include <glm/gtx/string_cast.hpp>

namespace renderer
{

    void Renderer::Render()
    {
        glClearColor(0.0f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthMask(GL_FALSE);
        skybox->Draw();
        glDepthMask(GL_TRUE);
        cull_lights();
        auto &layers = RenderLayerManager::GetInstance()->layers;
        for (auto &layer : layers)
            render(layer.GetQueue(OPAQUE), false);
    }

    void Renderer::cull_lights()
    {
        glUseProgram(light_culler->shader);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_totindex);
        glm::ivec2 st(0, 0);
        glBufferSubData(
            GL_SHADER_STORAGE_BUFFER,
            sizeof(int) * 65536 * 2,
            sizeof(glm::ivec2),
            glm::value_ptr(st));
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    }

    void Renderer::render(std::shared_ptr<render_queue_node> &now, bool include)
    {
        for (auto &obj : now->content.objects)
        {
            if (include || cam_param.Test(obj->args->box) != CameraParameters::FRUSTUM_SEPARATE)
            {
                obj->material->PrepareForDraw();
                obj->Draw(obj->material->shader->shader);
            }
        }

        if (now->subnodes[0] != nullptr)
        {
            for (auto &subnode : now->subnodes)
            {
                if (include)
                {
                    render(subnode, true);
                    continue;
                }
                CameraParameters::frustum_relation rel = cam_param.Test(subnode->box);

                if (rel == CameraParameters::FRUSTUM_INCLUDE)
                    render(subnode, true);
                else if (rel == CameraParameters::FRUSTUM_INTERSECT)
                    render(subnode, false);
            }
        }
    }

    CameraParameters::frustum_relation CameraParameters::Test(const common::BoundingBox &box)
    {
        glm::vec3 bmin = box.min;
        glm::vec3 bmax = box.max;
        glm::vec3 vertices[8];
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                for (int k = 0; k < 2; k++)
                {
                    glm::vec3 &vert = vertices[i * 4 + j * 2 + k];
                    vert.x = i ? bmin.x : bmax.x;
                    vert.y = j ? bmin.y : bmax.y;
                    vert.z = k ? bmin.z : bmax.z;
                }
        for (int i = 0; i < 8; i++)
            vertices[i] = view * glm::vec4(vertices[i], 1.0);

        bool allin, allout, intersect = false;
        for (int i = 0; i < 6; i++)
        {
            allin = true, allout = true;
            for (int j = 0; j < 8; j++)
            {
                glm::vec3 &vert = vertices[j];
                if (glm::dot(vert - frustum_pos[i], frustum_dir[i]) < 0)
                    allin = false;
                else
                    allout = false;
            }
            if (allout)
                return FRUSTUM_SEPARATE;
            intersect |= !allin;
        }

        if (intersect)
            return FRUSTUM_INTERSECT;

        return FRUSTUM_INCLUDE;
    }
}