#ifndef FLINT_TEXTURE_H
#define FLINT_TEXTURE_H

#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"

#include "rendering_server.h"

#include <memory>

class Texture {
public:
    /// Load a texture from directory.
    explicit Texture(const std::string &filename);
    ~Texture();

    uint32_t width = 0;
    uint32_t height = 0;

    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

    /// GPU memory.
    VkImage image{};

    /// CPU memory.
    VkDeviceMemory imageMemory{};

    /// Thin wrapper.
    VkImageView imageView;

    /// How image should be filtered.
    VkSampler sampler{};

    void createImageFromBytes(void *pixels, int texWidth, int texHeight);

    void cleanup() const;
};

#endif //FLINT_TEXTURE_H