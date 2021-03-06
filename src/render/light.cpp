#include "light.h"

namespace renderer
{
    LightManager *LightManager::instance = nullptr;

    light_id LightManager::InsertItem(std::shared_ptr<LightParameters> light)
    {
        light_id ret = ++maxid;
        switch (light->tp)
        {
        case POINT_LIGHT:
            if (pointlight_cnt == max_point_light)
                return 0;
            light->id = ret;
            light->index = pointlight_cnt;
            lights.insert(std::pair<light_id, std::shared_ptr<LightParameters>>(ret, light));
            inv_point_id.insert(std::pair<light_id, light_id>(pointlight_cnt, ret));
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_pointlights);
            send_lightdata(pointlight_cnt * 3 * sizeof(glm::vec4), light->inner_params);
            pointlight_cnt++;
            glBufferSubData(GL_UNIFORM_BUFFER, max_point_light * 3 * sizeof(glm::vec4), sizeof(int), &pointlight_cnt);
            break;
        case SPOT_LIGHT:
            if (spotlight_cnt == max_spot_light)
                return 0;
            light->id = ret;
            light->index = spotlight_cnt;
            lights.insert(std::pair<light_id, std::shared_ptr<LightParameters>>(ret, light));
            inv_spot_id.insert(std::pair<light_id, light_id>(spotlight_cnt, ret));
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_spotlights);
            send_lightdata(spotlight_cnt * 3 * sizeof(glm::vec4), light->inner_params);
            spotlight_cnt++;
            glBufferSubData(GL_UNIFORM_BUFFER, max_spot_light * 3 * sizeof(glm::vec4), sizeof(int), &spotlight_cnt);
            break;
        case DIRECTIONAL_LIGHT:
            if (directional_cnt == max_directional_light)
                return 0;
            light->id = ret;
            light->index = directional_cnt;
            lights.insert(std::pair<light_id, std::shared_ptr<LightParameters>>(ret, light));
            inv_directional_id.insert(std::pair<light_id, light_id>(directional_cnt, ret));
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_directionals);
            send_lightdata(directional_cnt * 3 * sizeof(glm::vec4), light->inner_params);
            directional_cnt++;
            glBufferSubData(GL_UNIFORM_BUFFER, max_directional_light * 3 * sizeof(glm::vec4), sizeof(int), &directional_cnt);
            break;
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        return ret;
    }

    void LightManager::UpdateItem(light_id id)
    {
        auto &param = lights[id];
        switch (param->tp)
        {
        case POINT_LIGHT:
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_pointlights);
            send_lightdata(param->index * 3 * sizeof(glm::vec4), param->inner_params);
            break;
        case SPOT_LIGHT:
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_spotlights);
            send_lightdata(param->index * 3 * sizeof(glm::vec4), param->inner_params);
            break;
        case DIRECTIONAL_LIGHT:
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_directionals);
            send_lightdata(param->index * 3 * sizeof(glm::vec4), param->inner_params);
            break;
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void LightManager::RemoveItem(light_id id)
    {
        auto &param = lights.find(id)->second;
        light_id idx;
        switch (param->tp)
        {
        case POINT_LIGHT:
            pointlight_cnt--;
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_pointlights);
            glBufferSubData(GL_UNIFORM_BUFFER, max_point_light * 3 * sizeof(glm::vec4), sizeof(int), &pointlight_cnt);
            if (!pointlight_cnt)
                break;
            idx = inv_point_id[pointlight_cnt];
            inv_point_id.erase(pointlight_cnt);
            inv_point_id[param->index] = idx;
            lights[idx]->index = param->index;
            send_lightdata(param->index * 3 * sizeof(glm::vec4), lights[idx]->inner_params);
            break;
        case SPOT_LIGHT:
            spotlight_cnt--;
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_spotlights);
            glBufferSubData(GL_UNIFORM_BUFFER, max_spot_light * 3 * sizeof(glm::vec4), sizeof(int), &spotlight_cnt);
            if (!spotlight_cnt)
                break;
            idx = inv_spot_id.find(spotlight_cnt)->second;
            inv_spot_id.erase(spotlight_cnt);
            inv_spot_id[param->index] = idx;
            lights[idx]->index = param->index;
            send_lightdata(param->index * 3 * sizeof(glm::vec4), lights[idx]->inner_params);
            break;
        case DIRECTIONAL_LIGHT:
            directional_cnt--;
            glBindBuffer(GL_UNIFORM_BUFFER, ubo_directionals);
            glBufferSubData(GL_UNIFORM_BUFFER, max_directional_light * 3 * sizeof(glm::vec4), sizeof(int), &directional_cnt);
            if (!directional_cnt)
                break;
            idx = inv_directional_id.find(directional_cnt)->second;
            inv_directional_id.erase(directional_cnt);
            inv_directional_id[param->index] = idx;
            lights[idx]->index = param->index;
            send_lightdata(param->index * 3 * sizeof(glm::vec4), lights[idx]->inner_params);
            break;
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        lights.erase(id);
    }
}