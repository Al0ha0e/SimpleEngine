#include "render_queue.h"

namespace renderer
{
    const int max_octlayer = 10;

    std::shared_ptr<RenderQueueIndex> RenderLayer::insert_obj(std::shared_ptr<render_queue_node> &now, RenderMode mode, std::shared_ptr<RenderQueueItem> &item)
    {
        auto &box = item->args->box;
        auto &objs = now->content.objects[mode];
        auto idx = std::make_shared<RenderQueueIndex>();
        if (now->depth >= max_octlayer)
            return insert_obj_to_node(now, mode, item, idx);
        push_tag(*now);
        int subnode = now->SubNodeTest(box);

        if (subnode != -1)
        {
            if (now->subnodes[0] == nullptr)
                split_node(now);
            return insert_obj(now->subnodes[subnode], mode, item);
        }
        return insert_obj_to_node(now, mode, item, idx);
    }

    void RenderLayer::RemoveObject(render_id id)
    {
        auto it = object_index.find(id);
        if (it == object_index.end())
            return;
        auto &index = it->second;

        auto &now = index->node;
        push_tag(*now);
        auto &content = now->content;
        content.objects[index->mode].erase(index->it);
        object_index.erase(it);

        std::shared_ptr<render_queue_node> combined = nullptr;
        for (auto &nd = now; nd != nullptr; nd = nd->father)
        {
            --nd->content.subtree_objcnt;
            if (!nd->content.subtree_objcnt)
                combined = nd;
        }
        if (combined != nullptr)
        {
            if (combined->father != nullptr)
                combined = combined->father;
            combine_node(combined);
        }
    }

    void RenderLayer::UpdateObject(render_id id)
    {
        auto it = object_index.find(id);
        if (it == object_index.end())
            return;
        auto &index = it->second;

        auto &item = *(index->it);

        auto &now = index->node;
        push_tag(*now);
        auto &box = item->args->box;
        if (now->box.LooseTest(box) == common::BOX_SEPARATE)
        {
            RemoveObject(id);
            InsertObject(index->mode, item);
        }
        if (now->depth >= max_octlayer)
            return;
        int subnode = now->SubNodeTest(box);
        if (subnode == -1)
            return;
        if (now->subnodes[0] == nullptr)
            now->Split();
        now->content.objects[index->mode].erase(index->it);
        object_index.erase(it);
        object_index[id] = insert_obj(now->subnodes[subnode], index->mode, item);
    }
}