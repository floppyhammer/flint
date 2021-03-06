#ifndef FLINT_SURFACE_H
#define FLINT_SURFACE_H

#include "material.h"
#include "../render/vertex.h"

#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"

#include <string>
#include <utility>
#include <vector>
#include <array>

namespace Flint {
    class Surface {
    public:
        Surface() = default;

        std::string name;

        VkBuffer get_vertex_buffer();

        VkBuffer get_index_buffer();

        uint32_t get_index_count();

        void set_gpu_resources(std::shared_ptr<VertexGpuResources<Vertex>> p_gpu_resources);

    private:
        std::shared_ptr<VertexGpuResources<Vertex>> gpu_resources;
    };

    class Surface2d : public Surface {
    public:
        Surface2d() = default;

        /**
         * Default quad surface.
         */
        static std::shared_ptr<Surface2d> from_default();

        void set_material(const std::shared_ptr<Material2d> &p_material);

        std::shared_ptr<Material2d> get_material() const;

    private:
        std::shared_ptr<Material2d> material;
    };

    class Surface3d : public Surface {
    public:
        Surface3d() = default;

        static std::shared_ptr<Surface3d> from_plane() {
            return std::make_shared<Surface3d>();
        }

        static std::shared_ptr<Surface3d> from_cube() {
            return std::make_shared<Surface3d>();
        }

        static std::shared_ptr<Surface3d> from_sphere() {
            return std::make_shared<Surface3d>();
        }

        void set_material(const std::shared_ptr<Material3d> &p_material);

        std::shared_ptr<Material3d> get_material() const;

    private:
        std::shared_ptr<Material3d> material;
    };
}

#endif //FLINT_SURFACE_H
