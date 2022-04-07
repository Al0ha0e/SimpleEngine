#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <map>
#include <memory>

#include "../common/ds.h"

namespace renderer
{
    typedef unsigned int light_id;
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
        glm::vec4 position;  // w: intensity
        glm::vec4 color;     // w: range
        glm::vec4 direction; // w: spot angle

        InnerLightParameters() {}
        InnerLightParameters(glm::vec4 position,
                             glm::vec4 color,
                             glm::vec4 direction)
            : position(position), color(color), direction(direction) {}
    };

    struct LightParameters
    {
        light_id id;
        light_id index;
        LightType tp;
        bool cast_shadow;
        InnerLightParameters inner_params;

        enum light_box_relation
        {
            LB_INCLUDE,
            LB_INTERSECT,
            LB_SEPARATE
        };

        LightParameters() {}
        LightParameters(LightType tp,
                        bool cast_shadow,
                        InnerLightParameters inner_params)
            : tp(tp), cast_shadow(cast_shadow), inner_params(inner_params) {}

        light_box_relation Test(const common::BoundingBox &box, float &impact)
        {
            bool hasin = false;
            bool hasout = false;
            glm::vec3 pos;
            glm::vec3 center = inner_params.position;
            glm::vec3 dir = inner_params.direction;
            float cutoff = inner_params.direction.w;
            float intensity = inner_params.position.w *
                              glm::length(glm::vec3(inner_params.color));
            impact = 0;
            float spimpact;

            for (int i = 0; i < 8; i++)
            {
                pos.x = i & 1 ? box.min.x : box.max.x;
                pos.y = (i >> 1) & 1 ? box.min.y : box.max.y;
                pos.z = (i >> 2) & 1 ? box.min.z : box.max.z;
                float dist = glm::distance(pos, center);
                float attenuation = 1.0 / (1 + 0.14 * dist + 0.07 * dist * dist);
                if (tp == POINT_LIGHT)
                {
                    spimpact = attenuation * intensity;
                    spimpact > 0.1 ? hasin = true : hasout = true;
                    impact += spimpact;
                }
                else
                {
                    float dirdet = glm::dot(
                        glm::normalize(center - pos),
                        glm::normalize(-dir));
                    float diratten = 1 - glm::clamp((cutoff - dirdet) / (0.2f * cutoff), 0.0f, 1.0f);
                    spimpact = diratten * attenuation * intensity;
                    spimpact > 0.1 ? hasin = true : hasout = true;
                    impact += spimpact;
                }
            }
            impact /= 8;
            return hasin ? (hasout ? LB_INTERSECT : LB_INCLUDE) : LB_SEPARATE;
        }
    };

    class LightManager
    {
    private:
        static LightManager *instance;
        ~LightManager() {} // TODO
        LightManager(const LightManager &);
        LightManager &operator=(const LightManager &);

    public:
        static LightManager *GetInstance()
        {
            if (instance == nullptr)
                instance = new LightManager();
            return instance;
        }

        light_id InsertItem(std::shared_ptr<LightParameters> light);

        void UpdateItem(light_id id);

        void RemoveItem(light_id id);

    private:
        light_id pointlight_cnt;
        light_id spotlight_cnt;
        light_id directional_cnt;
        light_id maxid;
        std::map<light_id, std::shared_ptr<LightParameters>> lights;
        std::map<light_id, light_id> inv_point_id;
        std::map<light_id, light_id> inv_spot_id;
        std::map<light_id, light_id> inv_directional_id;
        unsigned int ubo_pointlights;
        unsigned int ubo_spotlights;
        unsigned int ubo_directionals;

        LightManager() : pointlight_cnt(0), spotlight_cnt(0), directional_cnt(0), maxid(0)
        {
            glGenBuffers(1, &ubo_pointlights);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_pointlights);
            glBufferData(GL_UNIFORM_BUFFER, max_point_light * 3 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo_pointlights);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glGenBuffers(1, &ubo_spotlights);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_spotlights);
            glBufferData(GL_UNIFORM_BUFFER, max_spot_light * 3 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 3, ubo_spotlights);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glGenBuffers(1, &ubo_directionals);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_directionals);
            glBufferData(GL_UNIFORM_BUFFER, max_directional_light * 3 * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 4, ubo_directionals);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

        void send_lightdata(unsigned int offset, InnerLightParameters &param)
        {
            // std::cout << "OFFSET " << offset << std::endl;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(glm::vec4), glm::value_ptr(param.position));
            glBufferSubData(GL_UNIFORM_BUFFER, offset + sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(param.color));
            glBufferSubData(GL_UNIFORM_BUFFER, offset + 2 * sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(param.direction));
        }
    };
} // namespace renderer

#endif