#ifndef SCENE_H
#define SCENE_H

#include "game_object.h"

namespace common
{
    class Scene : public resources::SerializableObject
    {
    public:
        Scene() {}
        ~Scene() {}
        Scene(std::shared_ptr<renderer::Renderer> rd) : rd(rd) {}

        virtual void UnserializeJSON(std::string s)
        {
            auto j = nlohmann::json::parse(s);
            auto objs = j["objects"];
            for (auto &obj : objs)
            {
                auto object = std::make_shared<GameObject>(rd);
                object->self = object;
                object->UnserializeJSON(obj);
                objects.push_back(object);
            }
        }

        virtual std::string SerializeJSON()
        {
            std::string ret = "{\n";
            ret += "\"objects\": [";
            for (auto &obj : objects)
                ret += "\n" + obj->SerializeJSON() + ",";
            ret[ret.length() - 1] = ']';
            ret += "\n}";
            return ret;
        }

        void OnStart()
        {
            for (auto &object : objects)
            {
                object->OnStart();
            }
        }

        std::vector<std::shared_ptr<GameObject>> objects;

    private:
        std::shared_ptr<renderer::Renderer> rd;
    };
}

#endif