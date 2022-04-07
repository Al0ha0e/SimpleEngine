#ifndef RDQUEUE_H
#define RDQUEUE_H

#include "../common/common.h"
#include "light.h"
#include <list>

namespace renderer
{

    enum RenderMode
    {
        OPAQUE,
        TRANSPARENT,
        OPAQUE_SHADOW

    };

    typedef unsigned int render_id;

    struct RenderQueueItem
    {
        render_id id;
        std::shared_ptr<common::Material> material;
        std::shared_ptr<common::ModelMesh> mesh;
        std::shared_ptr<common::RenderArguments> args;
        unsigned int vertex_cnt;

        virtual void Draw(unsigned int shader_id)
        {
            mesh->PrepareForDraw();
            args->PrepareForDraw(shader_id);
            glDrawElements(GL_TRIANGLES, vertex_cnt, GL_UNSIGNED_INT, 0);
        }

        RenderQueueItem()
        {
            material = nullptr;
            mesh = nullptr;
            args = nullptr;
        }
        RenderQueueItem(unsigned int id,
                        std::shared_ptr<common::Material> material,
                        std::shared_ptr<common::ModelMesh> mesh,
                        std::shared_ptr<common::RenderArguments> args,
                        unsigned int vertex_cnt) : id(id), material(material), mesh(mesh), args(args), vertex_cnt(vertex_cnt) {}
    };

    typedef std::list<std::shared_ptr<RenderQueueItem>> ObjectList;
    typedef std::list<std::shared_ptr<LightParameters>> LightList;

    struct OctItem
    {
        int subtree_objcnt;
        ObjectList objects;
        LightList lights[2];

        void init()
        {
            subtree_objcnt = 0;
        }

        ~OctItem() {}
    };

    struct EmptyTag
    {
    };

    typedef common::OctNode<EmptyTag, OctItem> render_queue_node;

    struct RenderQueueLightIndex
    {
        std::shared_ptr<render_queue_node> node;
        LightList::iterator it;
        RenderQueueLightIndex() {}
    };

    struct RenderQueueIndex
    {
        std::shared_ptr<render_queue_node> node;
        ObjectList::iterator it;

        RenderQueueIndex()
        {
            node = nullptr;
        }

        RenderQueueIndex(std::shared_ptr<render_queue_node> &node,
                         ObjectList::iterator it) : node(node), it(it) {}
    };

    class RenderLayer
    {
    public:
        RenderLayer()
        {
            for (int i = 0; i < 3; i++)
            {
                // TODO
                render_queue[i] = std::make_shared<render_queue_node>(
                    common::BoundingBox(
                        glm::vec3(-1000.0, -1000.0, -1000.0),
                        glm::vec3(1000.0, 1000.0, 1000.0),
                        1.0),
                    0);
                render_queue[i]->content.init();
            }
        }

        void InsertObject(RenderMode mode, std::shared_ptr<RenderQueueItem> &item)
        {
            object_index[mode][item->id] = RenderQueueIndex();
            insert_obj(render_queue[mode], mode, item);
        }

        void RemoveObject(render_id id, RenderMode mode);
        void UpdateObject(render_id id, RenderMode mode);

        void InsertLight(std::shared_ptr<LightParameters> &light)
        {
        }

        void RemoveLight(light_id id)
        {
        }

        void UpdateLight(std::shared_ptr<LightParameters> &light)
        {
        }

        std::shared_ptr<render_queue_node> GetQueue(RenderMode mode)
        {
            return render_queue[mode];
        }

    private:
        std::shared_ptr<render_queue_node> render_queue[3];
        std::map<render_id, RenderQueueIndex> object_index[3];
        std::map<light_id, RenderQueueLightIndex> light_index;
        // TODO volatile light

        void insert_obj(std::shared_ptr<render_queue_node> &now, RenderMode mode, std::shared_ptr<RenderQueueItem> &item);

        void insert_light(std::shared_ptr<render_queue_node> &now, std::shared_ptr<LightParameters> &light);

        void insert_obj_to_node(
            std::shared_ptr<render_queue_node> &now,
            RenderMode mode,
            std::shared_ptr<RenderQueueItem> &item)
        {
            now->content.objects.push_back(item);
            for (auto nd = now; nd != nullptr; nd = nd->father)
                ++nd->content.subtree_objcnt;

            auto &idx = object_index[mode][item->id];
            idx.node = now;
            idx.it = now->content.objects.end();
            idx.it--;
        }

        void split_node(std::shared_ptr<render_queue_node> &now)
        {
            now->Split();
            for (int i = 0; i < 8; i++)
            {
                now->subnodes[i]->content.init();
                now->subnodes[i]->father = now;
            }
            // TODO push light
        }
    };

    struct RenderLayerIndex
    {
        unsigned char layer : 8;
        bool in_opaque : 1;
        bool in_shadow : 1;
        bool in_transparent : 1;

        RenderLayerIndex() {}
        RenderLayerIndex(unsigned char layer,
                         bool in_opaque,
                         bool in_shadow,
                         bool in_transparent)
            : layer(layer),
              in_opaque(in_opaque),
              in_shadow(in_shadow),
              in_transparent(in_transparent) {}
    };

    const int MAX_LAYER_NUM = 5;

    class RenderLayerManager
    {
    private:
        static RenderLayerManager *instance;

        ~RenderLayerManager() {} // TODO
        RenderLayerManager(const RenderLayerManager &);
        RenderLayerManager &operator=(const RenderLayerManager &);
        RenderLayerManager()
        {
        }

    public:
        static RenderLayerManager *GetInstance()
        {
            if (instance == nullptr)
                instance = new RenderLayerManager();
            return instance;
        }

        RenderLayer layers[MAX_LAYER_NUM];
    };
}

#endif