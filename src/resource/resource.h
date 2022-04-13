#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>
#include <map>
#include <memory>
#include <fstream>
#include "json.hpp"

namespace resources
{

    class SerializableObject
    {
    public:
        virtual std::string SerializeJSON() { return ""; }
        virtual void UnserializeJSON(std::string s) {}
        virtual void UnserializeJSON(nlohmann::json &j) {}
        virtual std::string SerializeBinary() { return ""; }
        virtual void UnserializeBinary(std::string s) {}
        virtual void Load(std::string pth) {}
    };

    extern std::map<std::string, std::shared_ptr<SerializableObject>> resource_in_memory;

    template <typename T>
    std::shared_ptr<T> LoadMeta(std::string pth)
    {
        auto it = resource_in_memory.find(pth);
        if (it != resource_in_memory.end())
            return std::static_pointer_cast<T>(it->second);
        std::shared_ptr<SerializableObject> obj = std::static_pointer_cast<SerializableObject>(std::make_shared<T>());
        std::ifstream infile(pth);
        std::stringstream buffer;
        buffer << infile.rdbuf();
        std::string s(buffer.str());
        infile.close();
        obj->UnserializeJSON(s);
        resource_in_memory[pth] = obj;
        return std::static_pointer_cast<T>(obj);
    }

    template <typename T>
    std::shared_ptr<T> Load(std::string pth)
    {
        auto it = resource_in_memory.find(pth);
        if (it != resource_in_memory.end())
            return std::static_pointer_cast<T>(it->second);
        std::shared_ptr<SerializableObject> obj = std::static_pointer_cast<SerializableObject>(std::make_shared<T>());
        obj->Load(pth);
        resource_in_memory[pth] = obj;
        return std::static_pointer_cast<T>(obj);
    }
}

#endif