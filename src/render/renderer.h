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
        glm::vec3 frustum_dir[6];
        glm::vec3 frustum_pos[6];

        enum frustum_relation
        {
            FRUSTUM_INCLUDE,
            FRUSTUM_INTERSECT,
            FRUSTUM_SEPARATE
        };

        frustum_relation Test(const common::BoundingBox &box);
        void UpdateParam(float nfov, float naspect, float nnear, float nfar)
        {
            fov = nfov;
            aspect = naspect;
            near = nnear;
            far = nfar;
            projection = glm::perspective(fov, aspect, near, far);
            float thfov = glm::tan(fov / 2) * aspect;
            for (int i = 0; i < 4; i++)
                frustum_pos[i] = glm::vec3(0.0f);
            frustum_pos[4] = glm::vec3(0, 0, -near);
            frustum_pos[5] = glm::vec3(0, 0, -far);
            frustum_dir[0] = glm::cross(glm::vec3(0, glm::tan(fov / 2), -1), glm::vec3(1.0, 0.0, 0.0));
            frustum_dir[1] = frustum_dir[0];
            frustum_dir[1].y = -frustum_dir[1].y;
            frustum_dir[2] = glm::cross(glm::vec3(0.0, 1.0, 0.0), glm::vec3(thfov, 0, -1));
            frustum_dir[3] = frustum_dir[2];
            frustum_dir[3].x = -frustum_dir[2].x;
            frustum_dir[4] = glm::vec3(0.0, 0.0, -1.0);
            frustum_dir[5] = glm::vec3(0.0, 0.0, 1.0);
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

        void UpdateView(const glm::mat4 &view, const glm::vec3 &viewPos, bool sub)
        {
            auto &param = sub ? sub_param : cam_param;
            param.view = view;
            param.viewPos = glm::vec4(viewPos, 0.0f);
            if (sub)
                return;
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_VP);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(param.view));
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::vec4), glm::value_ptr(param.viewPos));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        void UpdateProjection(float fov, float aspect, float near, float far, bool sub)
        {
            auto &param = sub ? sub_param : cam_param;
            param.UpdateParam(fov, aspect, near, far);
            if (sub)
                return;
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_VP);
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(param.projection));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

    private:
        CameraParameters cam_param;
        CameraParameters sub_param;
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