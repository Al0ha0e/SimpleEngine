#ifndef DS_H
#define DS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace common
{
    struct BoundingBox
    {
        glm::vec3 min;
        glm::vec3 max;
    };

    template <class Ttag, class Tcontent>
    struct OctNode
    {
        BoundingBox box;
        OctNode *subnodes[8];
        Ttag tag;
        Tcontent content;
    };
}

#endif