#include "sub_viewport.h"

#include "../../rendering/swap_chain.h"

#include <array>

namespace Flint {
    SubViewport::SubViewport() {
        type = NodeType::SubViewport;

        viewport = std::make_shared<Viewport>();
    }

    void SubViewport::_draw(VkCommandBuffer p_command_buffer) {
        // Begin render pass.
        {
            // It seems not feasible to wrap begin info into rendering Viewport.
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = viewport->renderPass;
            renderPassInfo.framebuffer = viewport->framebuffer; // Set target framebuffer.
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = VkExtent2D{viewport->get_extent().x, viewport->get_extent().y};

            // Clear color.
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {{clear_color.r, clear_color.g, clear_color.b, clear_color.a}};
            clearValues[1].depthStencil = {1.0f, 0};

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(p_command_buffer,
                                 &renderPassInfo,
                                 VK_SUBPASS_CONTENTS_INLINE);
        }

        // Start recursive calling to draw all node under this sub-viewport.
        Node::_draw(p_command_buffer);

        // End render pass.
        vkCmdEndRenderPass(p_command_buffer);
    }

    void SubViewport::_update(double delta) {
        Node::_update(delta);
    }

    std::shared_ptr<Texture> SubViewport::get_texture() const {
        return viewport->texture;
    }

    Vec2<uint32_t> SubViewport::get_extent() const {
        return viewport->get_extent();
    }
}
