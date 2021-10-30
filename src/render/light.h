#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <memory>

namespace renderer
{
    typedef unsigned long long light_id;
    const light_id max_directional_light = 8;
    const light_id max_point_light = 512;
    const light_id max_spot_light = 512;

    enum LightType
    {
        POINT_LIGHT,
        SPOT_LIGHT,
        DIRECTIONAL_LIGHT
    };

    struct InnerLightParameters
    {
        glm::vec4 position;  //w: intensity
        glm::vec4 color;     //w: range
        glm::vec4 direction; //w: spot angle
    };

    struct LightParameters
    {
        unsigned int index;
        LightType tp;
        bool cast_shadow;
        InnerLightParameters inner_params;
    };

    class LightManager
    {
    public:
        LightManager()
        {
            glGenBuffers(1, &ubo_pointlights);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_pointlights);
            glBufferData(GL_UNIFORM_BUFFER, max_point_light * 3 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_pointlights);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glGenBuffers(1, &ubo_spotlights);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_spotlights);
            glBufferData(GL_UNIFORM_BUFFER, max_spot_light * 3 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo_spotlights);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glGenBuffers(1, &ubo_directionals);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_directionals);
            glBufferData(GL_UNIFORM_BUFFER, max_directional_light * 3 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 3, ubo_directionals);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        light_id InsertItem(std::shared_ptr<LightParameters> light)
        {
            light_id ret = ++maxid;
            switch (light->tp)
            {
            case POINT_LIGHT:
                if (pointlight_cnt == max_point_light)
                    return 0;
                light->index = pointlight_cnt;
                lights.insert(std::pair<light_id, std::shared_ptr<LightParameters>>(ret, light));
                inv_point_id.insert(std::pair<unsigned int, light_id>(pointlight_cnt, ret));
                glBindBuffer(GL_UNIFORM_BUFFER, ubo_pointlights);
                send_lightdata(pointlight_cnt * 3 * sizeof(glm::vec4), light->inner_params);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                pointlight_cnt++;
                break;
            case SPOT_LIGHT:
                if (spotlight_cnt == max_spot_light)
                    return 0;
                light->index = spotlight_cnt;
                lights.insert(std::pair<light_id, std::shared_ptr<LightParameters>>(ret, light));
                inv_spot_id.insert(std::pair<unsigned int, light_id>(spotlight_cnt, ret));
                glBindBuffer(GL_UNIFORM_BUFFER, ubo_spotlights);
                send_lightdata(spotlight_cnt * 3 * sizeof(glm::vec4), light->inner_params);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                spotlight_cnt++;
                break;
            case DIRECTIONAL_LIGHT:
                if (directional_cnt == max_directional_light)
                    return 0;
                light->index = directional_cnt;
                lights.insert(std::pair<light_id, std::shared_ptr<LightParameters>>(ret, light));
                inv_directional_id.insert(std::pair<unsigned int, light_id>(directional_cnt, ret));
                glBindBuffer(GL_UNIFORM_BUFFER, ubo_directionals);
                send_lightdata(directional_cnt * 3 * sizeof(glm::vec4), light->inner_params);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                directional_cnt++;
                break;
            }
            return ret;
        }

        void RemoveItem(light_id id)
        {
            auto &param = lights.find(id)->second;
            switch (param->tp)
            {
            case POINT_LIGHT:
                unsigned int idx = inv_point_id[pointlight_cnt - 1];
                inv_point_id.erase(pointlight_cnt - 1);
                inv_point_id[param->index] = idx;
                lights[idx]->index = param->index;
                glBindBuffer(GL_UNIFORM_BUFFER, ubo_pointlights);
                send_lightdata(param->index * 3 * sizeof(glm::vec4), lights[idx]->inner_params);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                pointlight_cnt--;
                break;
            case SPOT_LIGHT:
                unsigned int idx = inv_spot_id.find(spotlight_cnt - 1)->second;
                inv_spot_id.erase(spotlight_cnt - 1);
                inv_spot_id[param->index] = idx;
                lights[idx]->index = param->index;
                glBindBuffer(GL_UNIFORM_BUFFER, ubo_spotlights);
                send_lightdata(param->index * 3 * sizeof(glm::vec4), lights[idx]->inner_params);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                spotlight_cnt--;
                break;
            case DIRECTIONAL_LIGHT:
                unsigned int idx = inv_directional_id.find(directional_cnt - 1)->second;
                inv_directional_id.erase(directional_cnt - 1);
                inv_directional_id[param->index] = idx;
                lights[idx]->index = param->index;
                glBindBuffer(GL_UNIFORM_BUFFER, ubo_directionals);
                send_lightdata(param->index * 3 * sizeof(glm::vec4), lights[idx]->inner_params);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                directional_cnt--;
                break;
            }
            lights.erase(id);
        }

    private:
        unsigned int pointlight_cnt;
        unsigned int spotlight_cnt;
        unsigned int directional_cnt;
        light_id maxid;
        std::map<light_id, std::shared_ptr<LightParameters>> lights;
        std::map<unsigned int, light_id> inv_point_id;
        std::map<unsigned int, light_id> inv_spot_id;
        std::map<unsigned int, light_id> inv_directional_id;
        unsigned int ubo_pointlights;
        unsigned int ubo_spotlights;
        unsigned int ubo_directionals;

        inline void send_lightdata(unsigned int offset, InnerLightParameters &param)
        {
            glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec4), glm::value_ptr(param.position));
            glBufferSubData(GL_UNIFORM_BUFFER, offset + sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(param.color));
            glBufferSubData(GL_UNIFORM_BUFFER, offset + 2 * sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(param.direction));
        }
    };

    //TODO Use generic to reduce it
    struct LightQueue
    {
        std::map<light_id, LightParameters> queue;
        light_id maxid;

        light_id GetLightID() { return ++maxid; }

        void InsertItem(light_id id, LightParameters item)
        {
            queue[id] = item;
        }

        void RemoveItem(light_id id)
        {
            if (queue.find(id) == queue.end())
                return;
            queue.erase(queue.find(id));
        }
    };
} // namespace renderer

#endif