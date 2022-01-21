#ifndef RDQUEUE_H
#define RDQUEUE_H

#include "../common/common.h"
#include "light.h"

namespace renderer
{

    struct RenderQueueItem
    {
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

        RenderQueueItem() {}
        RenderQueueItem(std::shared_ptr<common::Material> material,
                        std::shared_ptr<common::ModelMesh> mesh,
                        std::shared_ptr<common::RenderArguments> args,
                        unsigned int vertex_cnt) : material(material), mesh(mesh), args(args), vertex_cnt(vertex_cnt) {}
    };

    typedef std::vector<RenderQueueItem> RenderItemList;

    struct OctItem
    {
        RenderItemList objects;
        std::vector<std::shared_ptr<LightParameters>> lights;
    };

    struct OctTag
    {
        std::vector<std::shared_ptr<LightParameters>> lights;
        std::vector<int> del_lights;
    };

    struct EmptyTag
    {
    };

    class RenderLayer
    {
    public:
        common::OctNode<OctTag, OctItem> opaque_queue;
        common::OctNode<EmptyTag, RenderItemList> shadow_queue;
        common::OctNode<OctTag, OctItem> transparent_queue;
    };
}

#endif