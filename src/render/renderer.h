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
    struct CameraParameters
    {
        float fov;
        float aspect;
        float near;
        float far;
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 viewPos;

        enum frustum_relation
        {
            FRUSTUM_INCLUDE,
            FRUSTUM_INTERSECT,
            FRUSTUM_SEPARATE
        };

        frustum_relation Test(const common::BoundingBox &box)
        {
            return FRUSTUM_INCLUDE;
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

        void UpdateView(const glm::mat4 &view, const glm::vec3 &viewPos)
        {
            auto &param = cam_param;
            param.view = view;
            param.viewPos = glm::vec4(viewPos, 0.0f);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_VP);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(param.view));
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::vec4), glm::value_ptr(param.viewPos));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        void UpdateProjection(float fov, float aspect, float near, float far)
        {
            auto &param = cam_param;
            param.fov = fov;
            param.aspect = aspect;
            param.near = near;
            param.far = far;
            param.projection = glm::perspective(fov, aspect, near, far);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_VP);
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(param.projection));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

    private:
        CameraParameters cam_param;
        glm::vec4 ambient;
        unsigned int ubo_VP;
        unsigned int ubo_GI;
        unsigned int max_pointlight;
        unsigned int max_spotlight;

        std::shared_ptr<SkyBox> skybox;

        void render(std::shared_ptr<render_queue_node> &now, bool include);
    };
}

#endif