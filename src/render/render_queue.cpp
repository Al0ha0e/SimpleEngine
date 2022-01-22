#include "render_queue.h"

namespace renderer
{
    const int max_split_octlayer = 5;
    const int max_octlayer = 10;

    void RenderLayer::insert_obj(std::shared_ptr<node_with_light> now, std::shared_ptr<RenderQueueItem> &item, int depth)
    {
        auto &box = item->args->box;
        auto &objs = now->content.objects;
        if (now->subnodes[0] == nullptr)
        {
            //leaf
            if (objs.size() == 0)
            {
                if (depth >= max_octlayer || depth > max_split_octlayer)
                {
                    insert_obj_to_node(objs, item);
                }
                else
                {
                    int subnode = now->SubNodeTest(box);
                    if (subnode == -1)
                    {
                        insert_obj_to_node(objs, item);
                    }
                    else
                    {
                        now->Split();
                        insert_obj(now->subnodes[subnode], item, depth + 1);
                    }
                }
            }
            else
            {
                auto &pre = objs.back();
                int subnode = now->SubNodeTest(pre->args->box);
                if (subnode != -1)
                {
                    now->Split();
                    insert_obj(now->subnodes[subnode], pre, depth + 1);
                    objs.pop_back();
                    subnode = now->SubNodeTest(box);
                    subnode == -1 ? insert_obj(now->subnodes[subnode], item, depth + 1)
                                  : insert_obj_to_node(objs, item);
                }
                else
                {
                    now->Split();
                    subnode = now->SubNodeTest(box);
                    subnode == -1 ? insert_obj(now->subnodes[subnode], item, depth + 1)
                                  : insert_obj_to_node(objs, item);
                }
            }
        }
        else
        {
            push_tag(now);
            int subnode = now->SubNodeTest(box);
            if (subnode == -1)
            {
                insert_obj_to_node(objs, item);
            }
            else
            {
                insert_obj(now->subnodes[subnode], item, depth + 1);
            }
        }
    }
}