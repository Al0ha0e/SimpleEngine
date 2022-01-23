#include "render_queue.h"

namespace renderer
{
    const int max_octlayer = 10;

    std::shared_ptr<RenderQueueIndex> RenderLayer::insert_obj(std::shared_ptr<render_queue_node> &now, RenderMode mode, std::shared_ptr<RenderQueueItem> &item)
    {
        auto &box = item->args->box;
        auto &objs = now->content.objects[mode];
        if (now->depth >= max_octlayer)
            return insert_obj_to_node(now, mode, item);
        push_tag(*now);
        int subnode = now->SubNodeTest(box);

        if (subnode != -1)
        {
            if (now->subnodes[0] == nullptr)
                split_node(now);
            return insert_obj(now->subnodes[subnode], mode, item);
        }
        return insert_obj_to_node(now, mode, item);
    }

    void RenderLayer::RemoveObject(std::shared_ptr<RenderQueueIndex> &index, render_id id)
    {
        if ((*(index->it))->id != id)
            return;
        auto &now = index->node;
        push_tag(*now);
        auto &content = now->content;
        content.objects[index->mode].erase(index->it);
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

    std::shared_ptr<RenderQueueIndex> RenderLayer::UpdateObject(std::shared_ptr<RenderQueueIndex> &index, render_id id)
    {
        auto &item = *(index->it);
        if (item->id != id)
            return index;
        auto &now = index->node;
        push_tag(*now);
        auto &box = item->args->box;
        if (now->box.LooseTest(box) == common::BOX_SEPARATE)
        {
            RemoveObject(index, id);
            return InsertObject(index->mode, item);
        }
        if (now->depth >= max_octlayer)
            return index;
        int subnode = now->SubNodeTest(box);
        if (subnode == -1)
            return index;
        if (now->subnodes[0] == nullptr)
            now->Split();
        RemoveObject(index, id);
        return insert_obj(now->subnodes[subnode], index->mode, item);
    }
}