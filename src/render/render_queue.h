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

    struct RenderQueueLightItem
    {
        std::shared_ptr<LightParameters> param;
        float impact;
        bool inlazy;

        RenderQueueLightItem() {}
        RenderQueueLightItem(std::shared_ptr<LightParameters> param,
                             float impact,
                             bool inlazy) : param(param), impact(impact), inlazy(inlazy) {}
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
    typedef std::list<RenderQueueLightItem> LightList;

    struct OctItem
    {
        int subtree_objcnt;
        ObjectList objects[3];
        LightList lights_inclue[2];
        LightList lights_intersect[2];

        void init()
        {
            subtree_objcnt = 0;
        }
    };

    struct LightTag
    {
        LightList lights[2];
    };

    typedef common::OctNode<LightTag, OctItem> render_queue_node;

    struct RenderQueueLightIndex
    {
        LightType tp;
        std::map<std::shared_ptr<render_queue_node>, LightList::iterator> index;
        RenderQueueLightIndex() {}
        RenderQueueLightIndex(LightType tp) : tp(tp) {}
    };

    struct RenderQueueIndex
    {
        std::shared_ptr<render_queue_node> node;
        RenderMode mode;
        ObjectList::iterator it;

        RenderQueueIndex()
        {
            node = nullptr;
        }
        RenderQueueIndex(std::shared_ptr<render_queue_node> &node,
                         RenderMode mode,
                         ObjectList::iterator it) : node(node), mode(mode), it(it) {}
    };

    class RenderLayer
    {
    public:
        void InsertObject(RenderMode mode, std::shared_ptr<RenderQueueItem> &item)
        {
            object_index[item->id] = RenderQueueIndex();
            insert_obj(render_queue, mode, item);
        }

        void RemoveObject(render_id id);
        void UpdateObject(render_id id);

        void InsertLight(RenderMode mode, std::shared_ptr<LightParameters> &light)
        {
            light_index[light->id] = RenderQueueLightIndex(light->tp);
            LightList list;
            list.push_back(RenderQueueLightItem(light, 0, false));
            apply_tag(render_queue, list, light->tp);
        }

    private:
        std::shared_ptr<render_queue_node> render_queue;
        std::map<render_id, RenderQueueIndex> object_index;
        std::map<light_id, RenderQueueLightIndex> light_index;

        void insert_obj(std::shared_ptr<render_queue_node> &now, RenderMode mode, std::shared_ptr<RenderQueueItem> &item);

        void insert_obj_to_node(
            std::shared_ptr<render_queue_node> &now,
            RenderMode mode,
            std::shared_ptr<RenderQueueItem> &item)
        {
            now->content.objects[mode].push_back(item);
            for (auto &nd = now; nd != nullptr; nd = nd->father)
                ++nd->content.subtree_objcnt;
            auto &idx = object_index[item->id];
            idx.node = now;
            idx.mode = mode;
            idx.it = now->content.objects[mode].end();
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
            for (int i = 0; i < 2; i++)
            {
                if (now->content.lights_intersect[i].size())
                    for (auto &subnode : now->subnodes)
                        apply_tag(subnode, now->content.lights_intersect[i], LightType(i));
            }
            push_tag(*now);
        }

        void combine_node(std::shared_ptr<render_queue_node> &now)
        {
            for (int i = 0; i < 8; i++)
                now->subnodes[i] = nullptr;
        }

        LightList::iterator insert_light_to_list(LightList &list, std::shared_ptr<LightParameters> &param, float impact, bool inlazy)
        {
            auto it = list.begin();
            for (; it != list.end() && it->impact > impact; it++)
                ;
            return list.insert(it, RenderQueueLightItem(param, impact, inlazy));
        }

        void insert_light_to_index(light_id id, std::shared_ptr<render_queue_node> &now, LightList::iterator it)
        {
            light_index[id].index[now] = it;
        }

        void apply_tag(std::shared_ptr<render_queue_node> &now, LightList &lights, LightType tp)
        {
            for (auto &item : lights)
            {
                auto &param = item.param;
                auto rel = param->Test(now->box);
                if (rel == LB_SEPARATE)
                    continue;
                float impact = param->GetImpact(now->box);
                if (rel == LB_INCLUDE)
                {
                    auto it = insert_light_to_list(now->content.lights_inclue[tp], item.param, impact, false);
                    insert_light_to_index(param->id, now, it);
                    continue;
                }
                auto it = insert_light_to_list(now->content.lights_intersect[tp], item.param, impact, false);
                insert_light_to_index(param->id, now, it);
                if (now->subnodes[0] != nullptr)
                {
                    auto it = insert_light_to_list(now->tag.lights[tp], item.param, impact, true);
                    insert_light_to_index(param->id, now, it);
                }
            }
        }

        void push_tag(render_queue_node &now)
        {
            if (now.subnodes[0] == nullptr)
                return;
            if (now.tag.lights[0].size())
                for (auto &subnode : now.subnodes)
                    apply_tag(subnode, now.tag.lights[0], POINT_LIGHT);
            if (now.tag.lights[1].size())
                for (auto &subnode : now.subnodes)
                    apply_tag(subnode, now.tag.lights[1], SPOT_LIGHT);
        }
    };
}

#endif