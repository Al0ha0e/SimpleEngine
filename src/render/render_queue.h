#ifndef RDQUEUE_H
#define RDQUEUE_H

#include "../common/common.h"
#include "light.h"
#include <list>

namespace renderer
{

    typedef unsigned int render_id;
    struct RenderQueueItem;

    typedef std::list<std::shared_ptr<RenderQueueItem>> RenderItemList;

    struct RenderQueueItem
    {
        render_id id;
        std::shared_ptr<common::Material> material;
        std::shared_ptr<common::ModelMesh> mesh;
        std::shared_ptr<common::RenderArguments> args;
        unsigned int vertex_cnt;
        RenderItemList::iterator list_it;

        virtual void Draw(unsigned int shader_id)
        {
            mesh->PrepareForDraw();
            args->PrepareForDraw(shader_id);
            glDrawElements(GL_TRIANGLES, vertex_cnt, GL_UNSIGNED_INT, 0);
        }

        RenderQueueItem() {}
        RenderQueueItem(unsigned int id,
                        std::shared_ptr<common::Material> material,
                        std::shared_ptr<common::ModelMesh> mesh,
                        std::shared_ptr<common::RenderArguments> args,
                        unsigned int vertex_cnt) : id(id), material(material), mesh(mesh), args(args), vertex_cnt(vertex_cnt) {}
    };

    struct OctItem
    {
        RenderItemList objects;
        std::vector<std::shared_ptr<LightParameters>> lights;
    };

    struct EmptyTag
    {
    };

    struct LightTag
    {
        std::vector<std::shared_ptr<LightParameters>> lights;
        std::vector<int> del_lights;
    };

    typedef common::OctNode<LightTag, OctItem> node_with_light;
    typedef common::OctNode<EmptyTag, RenderItemList> node_without_light;

    class RenderLayer
    {
    public:
        std::shared_ptr<node_with_light> opaque_queue;
        std::shared_ptr<node_with_light> transparent_queue;
        std::shared_ptr<node_without_light> shadow_queue;

        void InsertObjectToOpaqueQueue()
        {
        }
        void InsertToShadowQueue()
        {
        }
        void InsertToTransparentQueue()
        {
        }

    private:
        void insert_obj(std::shared_ptr<node_with_light> now, std::shared_ptr<RenderQueueItem> &item, int depth);
        void insert_obj_to_node(RenderItemList &objs, std::shared_ptr<RenderQueueItem> &item)
        {
            objs.push_back(item);
            item->list_it = objs.end();
            item->list_it--;
        }
        void push_tag(std::shared_ptr<node_with_light> now)
        {
            //TODO
        }
    };
}

#endif