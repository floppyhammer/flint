#include "mesh_instance_3d.h"

#include "../../common/io.h"

#include <utility>

namespace Flint {
    const std::string MODEL_PATH = "../res/viking_room.obj";
    const std::string TEXTURE_PATH = "../res/viking_room.png";

    MeshInstance3D::MeshInstance3D() {
        createDescriptorPool();

        createDescriptorSets();

        // Load mesh resources.
        // --------------------------------
        // Load mesh.
        auto p_mesh = std::make_shared<Mesh>();
        p_mesh->loadFile(MODEL_PATH);
        set_mesh(p_mesh);

        // Load mesh texture.
        auto p_texture = std::make_shared<Texture>(TEXTURE_PATH);
        set_texture(p_texture);
        // --------------------------------
    }

    MeshInstance3D::~MeshInstance3D() {

    }

    void MeshInstance3D::set_mesh(std::shared_ptr<Mesh> p_mesh) {
        // Clean previous data.
        if (mesh != nullptr) {

        }

        mesh = std::move(p_mesh);

        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();

        vkResourcesAllocated = true;
    }

    std::shared_ptr<Mesh> MeshInstance3D::get_mesh() const {
        return mesh;
    }

    void MeshInstance3D::set_texture(std::shared_ptr<Texture> p_texture) {
        texture = std::move(p_texture);

        updateDescriptorSets();
    }

    void MeshInstance3D::update(double delta) {
        Node3D::update(delta);
    }

    // This will be called by the scene tree.
    void MeshInstance3D::draw() {
        Node3D::draw();

        if (mesh == nullptr || texture == nullptr) return;

        VkBuffer vertexBuffers[] = {vertexBuffer};
        RS::getSingleton().draw_mesh_instance(descriptorSets[RS::getSingleton().p_currentImage],
                                              vertexBuffers,
                                              indexBuffer,
                                              mesh->indices.size());
    }

    // Create descriptor pool before creating descriptor sets.
    void MeshInstance3D::createDescriptorPool() {
        auto swapChainImages = RS::getSingleton().p_swapChainImages;

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages->size());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages->size());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(swapChainImages->size());

        if (vkCreateDescriptorPool(Device::getSingleton().device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor pool!");
        }
    }

    void MeshInstance3D::createDescriptorSets() {
        auto swapChainImages = RS::getSingleton().p_swapChainImages;
        auto &descriptorSetLayout = RS::getSingleton().meshInstance3dDescriptorSetLayout;
        auto device = Device::getSingleton().device;

        std::vector<VkDescriptorSetLayout> layouts(swapChainImages->size(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages->size());
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(swapChainImages->size());
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }
    }

    void MeshInstance3D::updateDescriptorSets() {
        auto swapChainImages = RS::getSingleton().p_swapChainImages;
        auto &descriptorSetLayout = RS::getSingleton().meshInstance3dDescriptorSetLayout;
        auto device = Device::getSingleton().device;

        for (size_t i = 0; i < swapChainImages->size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture->imageView;
            imageInfo.sampler = texture->sampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            // Update the contents of a descriptor set object.
            vkUpdateDescriptorSets(device,
                                   static_cast<uint32_t>(descriptorWrites.size()),
                                   descriptorWrites.data(),
                                   0,
                                   nullptr);
        }
    }
}
