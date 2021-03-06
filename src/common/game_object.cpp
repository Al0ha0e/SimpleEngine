#include "game_object.h"

namespace common
{
    unsigned int GameObject::object_cnt = 0;

    std::shared_ptr<Component> UnserializeComponent(
        std::string tp,
        nlohmann::json &j,
        std::shared_ptr<GameObject> obj)
    {
        if (tp == "renderable_obj")
        {
            auto ret = std::make_shared<builtin_components::RenderableObject>();
            ret->UnserializeJSON(j, obj);
            return ret;
        }
        else if (tp == "camera")
        {
            auto ret = std::make_shared<builtin_components::Camera>();
            ret->UnserializeJSON(j, obj);
            return ret;
        }
        else if (tp == "light")
        {
            auto ret = std::make_shared<builtin_components::Light>();
            ret->UnserializeJSON(j, obj);
            return ret;
        }
        return std::make_shared<Component>();
    }

    std::string SerializeComponent(Component &component)
    {
        std::string ret = "{\n";
        ret += "\"tp\": ";
        if (typeid(component) == typeid(builtin_components::Camera))
        {
            ret += "\"camera\"";
        }
        else if (typeid(component) == typeid(builtin_components::Light))
        {
            ret += "\"light\"";
        }
        else if (typeid(component) == typeid(builtin_components::RenderableObject))
        {
            ret += "\"renderable_obj\"";
        }
        ret += ",\n\"content\":" + component.SerializeJSON();
        ret += "\n}";
        return ret;
    }
}