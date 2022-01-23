#ifndef DS_H
#define DS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>

namespace common
{

    inline bool Vec3LE(glm::vec3 &a, glm::vec3 &b)
    {
        return a.x <= b.x && a.y <= b.y && a.z <= b.z;
    }

    enum BoxRelation
    {
        BOX_INCLUDE,
        BOX_INV_INCLUDE,
        BOX_INTERSECT,
        BOX_SEPARATE
    };

    struct BoundingBox
    {
        glm::vec3 min;
        glm::vec3 max;
        glm::vec3 loose_min;
        glm::vec3 loose_max;
        float loose_factor;

        BoundingBox() {}

        BoundingBox(glm::vec3 min, glm::vec3 max, float l) : min(min), max(max), loose_factor(l)
        {
            GetLoosedBounding(min, max, loose_min, loose_max, l);
        }

        static void GetLoosedBounding(glm::vec3 min, glm::vec3 max, glm::vec3 &lmin, glm::vec3 &lmax, float l)
        {
            lmin = min * 0.5f * (1 + l) + max * 0.5f * (1 - l); //(min + max) * 0.5f - (min + max) * 0.5f * l + min *l;
            lmax = min * 0.5f * (1 - l) + max * 0.5f * (1 + l); //(min + max) * 0.5f + (min + max) * 0.5f * l - min *l;
        }

        BoxRelation Test(BoundingBox &ano)
        {
            glm::vec3 &min1 = ano.min;
            glm::vec3 &max1 = ano.max;
            if (Vec3LE(min, min1) && Vec3LE(max1, max))
                return BOX_INCLUDE;
            if (Vec3LE(min1, min) && Vec3LE(max, max1))
                return BOX_INV_INCLUDE;
            if (min.x >= max1.x || min.y >= max1.y || min.z >= max1.z)
                return BOX_SEPARATE;
            if (min1.x >= max.x || min1.y >= max.y || min1.z >= max.z)
                return BOX_SEPARATE;
            return BOX_INTERSECT;
        }

        BoxRelation LooseTest(BoundingBox &ano)
        {
            glm::vec3 &min1 = ano.min;
            glm::vec3 &max1 = ano.max;
            if (Vec3LE(loose_min, min1) && Vec3LE(max1, loose_max))
                return BOX_INCLUDE;
            if (Vec3LE(min1, min) && Vec3LE(max, max1))
                return BOX_INV_INCLUDE;
            if (min.x >= max1.x || min.y >= max1.y || min.z >= max1.z)
                return BOX_SEPARATE;
            if (min1.x >= max.x || min1.y >= max.y || min1.z >= max.z)
                return BOX_SEPARATE;
            return BOX_INTERSECT;
        }
    };

    //TODO memory management
    template <class Ttag, class Tcontent>
    struct OctNode
    {
        unsigned int depth;
        std::shared_ptr<OctNode<Ttag, Tcontent>> subnodes[8];
        std::shared_ptr<OctNode<Ttag, Tcontent>> father;
        BoundingBox box;
        Ttag tag;
        Tcontent content;

        OctNode()
        {
            for (int i = 0; i < 8; i++)
                subnodes[i] = nullptr;
            father = nullptr;
            depth = 0;
        }
        OctNode(BoundingBox box, unsigned int depth) : box(box), depth(depth)
        {
            for (int i = 0; i < 8; i++)
                subnodes[i] = nullptr;
            father = nullptr;
        }

        int SubNodeTest(BoundingBox &ano)
        {
            if (box.Test(ano) == BOX_INV_INCLUDE)
                return -1;
            glm::vec3 &min = ano.min;
            glm::vec3 &max = ano.max;
            int idx = 0;
            if (subnodes[0] != nullptr)
            {
                idx += max.x <= subnodes[0]->box.max.x ? 0 : 4;
                idx += max.y <= subnodes[0]->box.max.y ? 0 : 2;
                idx += max.z <= subnodes[0]->box.max.z ? 0 : 1;
                if (subnodes[idx]->box.LooseTest(ano) == BOX_INCLUDE)
                    return idx;
                return -1;
            }
            glm::vec3 &bmin = box.min;
            glm::vec3 &bmax = box.max;
            glm::vec3 mid = (bmin + bmax) * 0.5f;
            glm::vec3 lmin, lmax;
            BoundingBox::GetLoosedBounding(bmin, mid, lmin, lmax, box.loose_factor);
            glm::vec3 nmin, nmax;
            if (max.x <= lmax.x)
            {
                nmin.x = bmin.x;
                nmax.x = mid.x;
            }
            else
            {
                idx += 4;
                nmin.x = mid.x;
                nmax.x = bmax.x;
            }
            if (max.y <= lmax.y)
            {
                nmin.y = bmin.y;
                nmax.y = mid.y;
            }
            else
            {
                idx += 2;
                nmin.y = mid.y;
                nmax.y = bmax.y;
            }
            if (max.z <= lmax.z)
            {
                nmin.z = bmin.z;
                nmax.z = mid.z;
            }
            else
            {
                idx += 1;
                nmin.z = mid.z;
                nmax.z = bmax.z;
            }
            if (BoundingBox(nmin, nmax, box.loose_factor).LooseTest(ano) == BOX_INCLUDE)
                return idx;
            return -1;
        }

        void Split()
        {
            auto &min = box.min;
            auto &max = box.max;
            float x[3] = {min.x, (min.x + max.x) * 0.5f, max.x};
            float y[3] = {min.y, (min.y + max.y) * 0.5f, max.y};
            float z[3] = {min.z, (min.z + max.z) * 0.5f, max.z};
            float f = box.loose_factor;
            for (int i = 0; i < 2; i++)
                for (int j = 0; j < 2; j++)
                    for (int k = 0; k < 2; k++)
                        subnodes[(i << 2) | (j << 1) | k] = std::make_shared<OctNode<Ttag, Tcontent>>(
                            BoundingBox(glm::vec3(x[i], y[j], z[k]), glm::vec3(x[i + 1], y[j + 1], z[k + 1]), f),
                            depth + 1);
        }

        void Dispose() {}
    };
}

#endif