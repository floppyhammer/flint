#ifndef FLINT_SKYBOX_H
#define FLINT_SKYBOX_H

#include "node_3d.h"
#include "../../../resources/mesh.h"
#include "../../../resources/cubemap_texture.h"

#include "glm/glm.hpp"

namespace Flint {
    class Skybox : public Node3D {
    public:
        Skybox();

    protected:
        void draw(VkCommandBuffer p_command_buffer) override;

        std::shared_ptr<MaterialSkybox> material;

        std::shared_ptr<VertexGpuResources<SkyboxVertex>> surface_gpu_resources;
    };
}

#endif //FLINT_SKYBOX_H
