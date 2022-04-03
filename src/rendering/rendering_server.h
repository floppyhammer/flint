#ifndef FLINT_RENDERING_SERVER_H
#define FLINT_RENDERING_SERVER_H

#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"

#include "device.h"

#include <vector>
#include <iostream>
#include <optional>

class RenderingServer {
public:
    static RenderingServer &getSingleton() {
        static RenderingServer singleton;
        return singleton;
    }

    RenderingServer();

    void createSwapChainRelatedResources(VkRenderPass renderPass, VkExtent2D swapChainExtent);

    [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

public:
    void cleanup();

    /**
     * Create a shader module, but the shader stage is not specified yet.
     * @return Shader module.
     */
    [[nodiscard]] static VkShaderModule createShaderModule(const std::vector<char> &code);

    /**
     * Create a command buffer in the command pool, and start recording.
     * @return Command buffer.
     */
    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const;

    /**
     * End recording, submit the command buffer to a queue, then free the command buffer in the command pool.
     */
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;

    /**
     * Copy data from VkBuffer to VkImage.
     * @param buffer Src buffer.
     * @param image Dst image.
     * @param width Image width.
     * @param height Image height.
     */
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;

    static bool hasStencilComponent(VkFormat format);

    /**
     * Copy data from VkBuffer to VkBuffer.
     * @param srcBuffer Src buffer.
     * @param dstBuffer Dst buffer.
     * @param VkDeviceSize Data size in bytes.
     */
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) const;

    /**
     * An image view is a reference to a VkImage.
     * Unlike VkImage, it does not need to be allocated on GPU memory,
     * so you create them directly from the Vulkan API.
     * @param image
     * @param format
     * @param aspectFlags If for color or depth attachment.
     * @return
     */
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

    /**
     * Create GPU buffer and CPU buffer memory and bind them.
     * @param size
     * @param usage
     * @param properties
     * @param buffer
     * @param bufferMemory
     */
    void createBuffer(VkDeviceSize size,
                      VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkBuffer &buffer,
                      VkDeviceMemory &bufferMemory) const;

    /**
     * Copy data to device memory.
     * @param src Source data pointer.
     * @param bufferMemory Device memory.
     * @param dataSize Data size in bytes.
     */
    void copyDataToMemory(void *src, VkDeviceMemory bufferMemory, size_t dataSize) const;

    void createTextureSampler(VkSampler &textureSampler) const;

public:
    // Texture rect.
    // --------------------------------------------------

    // --------------------------------------------------

    // Mesh instance 3D.
    // --------------------------------------------------
    // These should be shared by all mesh instances.
    VkDescriptorSetLayout meshDescriptorSetLayout;
    VkPipelineLayout meshPipelineLayout;
    VkPipeline meshGraphicsPipeline;

    VkDescriptorSetLayout blitDescriptorSetLayout;
    VkPipelineLayout blitPipelineLayout;
    VkPipeline blitGraphicsPipeline;

    /**
     * Create a descriptor set layout for mesh drawing.
     * @dependency None.
     */
    void createMeshDescriptorSetLayout();

    /**
     * Create pipeline layout. Must do this before creating pipeline.
     * @param descriptorSetLayout Descriptor set layout of the pipeline.
     * @param graphicsPipelineLayout Output.
     * dependency VkDescriptorSetLayout.
     */
    void createGraphicsPipelineLayout(VkDescriptorSetLayout &descriptorSetLayout,
                                      VkPipelineLayout &graphicsPipelineLayout);

    /**
     * Set up shaders, viewport, blend state, etc.
     * @param renderPass Target render pass.
     * @param viewportExtent Target viewport size.
     * @param graphicsPipeline Generated pipeline.
     * @note We only need one pipeline for a specific rendering process despite of the existence of multiple swap chains.
     * @dependency Descriptor set layout, render pass, viewport extent.
     */
    void createMeshGraphicsPipeline(VkRenderPass renderPass,
                                    VkExtent2D viewportExtent,
                                    VkPipeline &graphicsPipeline);

    /**
     * Draw a single mesh.
     * @param commandBuffer
     * @param meshGraphicsPipeline
     * @param descriptorSet
     * @param vertexBuffers
     * @param indexBuffer
     * @param indexCount
     */
    void draw_mesh(VkCommandBuffer commandBuffer,
                   VkPipeline meshGraphicsPipeline,
                   VkDescriptorSet const &descriptorSet,
                   VkBuffer *vertexBuffers,
                   VkBuffer indexBuffer,
                   uint32_t indexCount) const;

    void blit(VkCommandBuffer commandBuffer, VkPipeline graphicsPipeline, VkDescriptorSet const &descriptorSet,
              VkBuffer *vertexBuffers, VkBuffer indexBuffer, uint32_t indexCount) const;

    // --------------------------------------------------
    void cleanupSwapChainRelatedResources() const;

    void createCommandPool();

    VkCommandPool commandPool;

    void createBlitGraphicsPipeline(VkRenderPass renderPass, VkExtent2D viewportExtent, VkPipeline &graphicsPipeline);

    void createBlitDescriptorSetLayout();
};

typedef RenderingServer RS;

#endif //FLINT_RENDERING_SERVER_H
