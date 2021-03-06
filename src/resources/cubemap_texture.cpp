#include "cubemap_texture.h"
#include "../common/logger.h"
#include "../common/macros.h"

// Already defined in Pathfinder.
//#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include <stdexcept>

namespace Flint {
    CubemapTexture::CubemapTexture(const std::string &path) {
        load_from_file(path);
    }

    void CubemapTexture::load_from_file(const std::string &path) {
        auto device = Platform::getSingleton()->device;

        // TODO: Add mip levels.
        uint32_t mipLevels = 1;

        int tex_width, tex_height, tex_channels;
        stbi_uc *pixels = stbi_load(path.c_str(),
                                    &tex_width,
                                    &tex_height,
                                    &tex_channels,
                                    STBI_rgb_alpha);

        if (!pixels) {
            Flint::Logger::warn("Failed to load image file: " + path, "Texture");
            throw std::runtime_error("Failed to load texture image!");
        }

        width = tex_width;
        height = tex_height / 6;

        format = VK_FORMAT_R8G8B8A8_UNORM;

        // Image layer data size per layer in bytes.
        VkDeviceSize layer_data_size = width * height * 4;

        // Temporary buffer and CPU memory.
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        auto rs = RenderServer::getSingleton();

        rs->createBuffer(layer_data_size * 6,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         staging_buffer,
                         staging_buffer_memory);

        // Copy the pixel values that we got from the image loading library to the buffer.
        rs->copyDataToMemory(pixels,
                             staging_buffer_memory,
                             layer_data_size * 6,
                             0);

        // Clean up the original pixel array.
        stbi_image_free(pixels);

        // Create optimal tiled target image.
        rs->createImage(width,
                        height,
                        format,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        image,
                        imageMemory,
                        6,
                        VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

        // Setup buffer copy regions for each face including all of its mip levels.
        std::vector<VkBufferImageCopy> buffer_copy_regions;

        // Order has to be (+X, -X, +Y, -Y, +Z, -Z).
        for (uint32_t face = 0; face < 6; face++) {
            // Mip level has to be 1 for now.
            for (uint32_t level = 0; level < mipLevels; level++) {
                // Calculate offset into staging buffer for the current mip level and face
                size_t offset = face * layer_data_size;

                VkBufferImageCopy buffer_copy_region = {};
                buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                buffer_copy_region.imageSubresource.mipLevel = level;
                buffer_copy_region.imageSubresource.baseArrayLayer = face;
                buffer_copy_region.imageSubresource.layerCount = 1;
                buffer_copy_region.imageExtent.width = width >> level;
                buffer_copy_region.imageExtent.height = height >> level;
                buffer_copy_region.imageExtent.depth = 1;
                buffer_copy_region.bufferOffset = offset;
                buffer_copy_regions.push_back(buffer_copy_region);
            }
        }

        // Image barrier for optimal image (target).
        // Set initial layout for all array layers (faces) of the optimal (target) tiled texture.
        rs->transitionImageLayout(image,
                                  format,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  1,
                                  6);

        // Copy the cube map faces from the staging buffer to the optimal tiled image.
        VkCommandBuffer command_buffer = rs->beginSingleTimeCommands();

        vkCmdCopyBufferToImage(
                command_buffer,
                staging_buffer,
                image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                static_cast<uint32_t>(buffer_copy_regions.size()),
                buffer_copy_regions.data()
        );

        rs->endSingleTimeCommands(command_buffer);

        // Change texture image layout to shader read after all faces have been copied.
        rs->transitionImageLayout(image,
                                  format,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  1,
                                  6);

        // Create sampler.
        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeV = sampler_info.addressModeU;
        sampler_info.addressModeW = sampler_info.addressModeU;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.compareOp = VK_COMPARE_OP_NEVER;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = mipLevels;
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        sampler_info.maxAnisotropy = 1.0f;

        VK_CHECK_RESULT(vkCreateSampler(device, &sampler_info, nullptr, &sampler));

        // Create image view.
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        view_info.format = format;
        view_info.components = {VK_COMPONENT_SWIZZLE_R,
                                VK_COMPONENT_SWIZZLE_G,
                                VK_COMPONENT_SWIZZLE_B,
                                VK_COMPONENT_SWIZZLE_A};
        view_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        // 6 array layers (faces).
        view_info.subresourceRange.layerCount = 6;
        // Set number of mip levels.
        view_info.subresourceRange.levelCount = mipLevels;
        view_info.image = image;

        VK_CHECK_RESULT(vkCreateImageView(device, &view_info, nullptr, &imageView));
    }
}
