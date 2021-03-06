#ifndef FLINT_NODE_3D_H
#define FLINT_NODE_3D_H

#include "../node.h"
#include "../../../render/render_server.h"
#include "../../../resources/surface.h"
#include "../../../render/mvp.h"

#include "glm/glm.hpp"
//#include <Eigen/Geometry>

namespace Flint {
    class Node3D : public Node {
    public:
        Node3D();

        ~Node3D();

        // Transform.
        // ------------------------------------------
        glm::vec3 position{0};

        glm::vec3 rotation{0};
        //Eigen::Quaternionf quaternion;

        glm::vec3 scale{1};
        // ------------------------------------------

    protected:
        void update(double delta) override;

        void update_mvp();

        MvpPushConstant push_constant;
    };
}

#endif //FLINT_NODE_3D_H
