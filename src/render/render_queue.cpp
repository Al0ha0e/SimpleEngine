#include "render_queue.h"

namespace renderer
{
    const int max_octlayer = 10;

    void RenderLayer::insert_obj(std::shared_ptr<render_queue_node> &now, RenderMode mode, std::shared_ptr<RenderQueueItem> &item)
    {
        auto &box = item->args->box;
        auto &objs = now->content.objects;
        if (now->depth >= max_octlayer)
            return insert_obj_to_node(now, mode, item);
        int subnode = now->SubNodeTest(box);

        if (subnode != -1)
        {
            if (now->subnodes[0] == nullptr)
                split_node(now);
            return insert_obj(now->subnodes[subnode], mode, item);
        }
        insert_obj_to_node(now, mode, item);
    }

    void RenderLayer::RemoveObject(render_id id, RenderMode mode)
    {
        auto &obj_idxs = object_index[mode];
        auto it = obj_idxs.find(id);
        if (it == obj_idxs.end())
            return;

        auto &index = it->second;
        auto &now = index.node;
        auto &content = now->content;
        content.objects.erase(index.it);
        obj_idxs.erase(it);

        std::shared_ptr<render_queue_node> combined = nullptr;
        for (auto &nd = now; nd != nullptr; nd = nd->father)
        {
            --nd->content.subtree_objcnt;
            if (!nd->content.subtree_objcnt)
                combined = nd;
        }
        if (combined != nullptr && now->subnodes[0] != nullptr)
            for (int i = 0; i < 8; i++)
                now->subnodes[i] = nullptr;
    }

    void RenderLayer::UpdateObject(render_id id, RenderMode mode)
    {
        auto &obj_idxs = object_index[mode];
        auto it = obj_idxs.find(id);

        if (it == obj_idxs.end())
            return;
        auto &index = it->second;
        auto &item = *(index.it);
        auto &now = index.node;
        auto &box = item->args->box;
        if (now->box.LooseTest(box) != common::BoundingBox::BOX_INCLUDE)
        {
            RemoveObject(id, mode);
            InsertObject(mode, item);
            return;
        }
        if (now->depth >= max_octlayer)
            return;
        int subnode = now->SubNodeTest(box);
        if (subnode == -1)
            return;
        if (now->subnodes[0] == nullptr)
            split_node(now);

        for (auto &nd = now; nd != nullptr; nd = nd->father)
            --nd->content.subtree_objcnt;

        now->content.objects.erase(index.it);
        insert_obj(now->subnodes[subnode], mode, item);
    }

    void RenderLayer::insert_light(std::shared_ptr<render_queue_node> &now, std::shared_ptr<LightParameters> &light)
    {
    }

    RenderLayerManager *RenderLayerManager::instance = nullptr;
}