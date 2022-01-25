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
        OPAQUE_SHADOW,
        TRANSPARENT
    };

    struct LightInRenderQueue
    {
        std::shared_ptr<LightParameters> param;
        float impact;
    };

    typedef unsigned int render_id;
    struct RenderQueueItem;

    typedef std::list<std::shared_ptr<RenderQueueItem>> RenderItemList;
    typedef std::list<LightInRenderQueue> LightList;

    struct OctItem
    {
        int subtree_objcnt;
        RenderItemList objects[3];
        LightList lights;

        void init()
        {
            subtree_objcnt = 0;
        }
    };

    struct LightTag
    {
        LightList lights;
        std::vector<int> del_lights;
    };

    typedef common::OctNode<LightTag, OctItem> render_queue_node;

    struct RenderQueueIndex
    {
        std::shared_ptr<render_queue_node> node;
        RenderMode mode;
        RenderItemList::iterator it;

        RenderQueueIndex()
        {
            node = nullptr;
        }
        RenderQueueIndex(std::shared_ptr<render_queue_node> &node,
                         RenderMode mode,
                         RenderItemList::iterator it) : node(node), mode(mode), it(it) {}
    };

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

    class RenderLayer
    {
    public:
        void InsertObject(RenderMode mode, std::shared_ptr<RenderQueueItem> &item)
        {
            object_index[item->id] = insert_obj(render_queue, mode, item);
        }

        void RemoveObject(render_id id);
        void UpdateObject(render_id id);

    private:
        std::shared_ptr<render_queue_node> render_queue;
        std::map<render_id, std::shared_ptr<RenderQueueIndex>> object_index;

        std::shared_ptr<RenderQueueIndex> insert_obj(std::shared_ptr<render_queue_node> &now, RenderMode mode, std::shared_ptr<RenderQueueItem> &item);

        std::shared_ptr<RenderQueueIndex> insert_obj_to_node(
            std::shared_ptr<render_queue_node> &now,
            RenderMode mode,
            std::shared_ptr<RenderQueueItem> &item,
            std::shared_ptr<RenderQueueIndex> &idx)
        {
            now->content.objects[mode].push_back(item);
            for (auto &nd = now; nd != nullptr; nd = nd->father)
                ++nd->content.subtree_objcnt;
            idx->node = now;
            idx->mode = mode;
            idx->it = now->content.objects[mode].end();
            idx->it--;
            return idx;
        }

        void split_node(std::shared_ptr<render_queue_node> &now)
        {
            now->Split();
            for (int i = 0; i < 8; i++)
            {
                now->subnodes[i]->content.init();
                now->subnodes[i]->father = now;
            }
        }

        void combine_node(std::shared_ptr<render_queue_node> &now)
        {
            for (int i = 0; i < 8; i++)
                now->subnodes[i] = nullptr;
        }

        void push_tag(render_queue_node &now)
        {
            //TODO
        }
    };
}

#endif