#ifndef FLINT_IMAGE_TEXTURE_H
#define FLINT_IMAGE_TEXTURE_H

#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"

#include "resource.h"
#include "../render/render_server.h"

#include <memory>

class ImageTexture : public Resource {
public:
    ImageTexture() = default;

    /// Create a texture from file.
    explicit ImageTexture(const std::string &path);

    ~ImageTexture() override;

    /// Create an empty texture with specific size.
    static std::shared_ptr<ImageTexture> from_empty(uint32_t p_width, uint32_t p_height);

    //static std::shared_ptr<Texture> from_bytes(const std::string &filename);

    bool resource_ownership = true;

public:
    uint32_t width = 0;
    uint32_t height = 0;

    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

    /// GPU memory.
    VkImage image{};

    /// CPU memory.
    VkDeviceMemory imageMemory{};

    /// Thin wrapper.
    VkImageView imageView{};

    /// How image should be filtered.
    VkSampler sampler{};

private:
    void create_image_from_bytes(void *pixels, uint32_t tex_width, uint32_t tex_height);
};

#endif //FLINT_IMAGE_TEXTURE_H