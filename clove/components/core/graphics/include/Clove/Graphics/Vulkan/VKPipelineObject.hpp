#pragma once

#include "Clove/Graphics/PipelineObject.hpp"
#include "Clove/Graphics/Vulkan/DevicePointer.hpp"

#include <vulkan/vulkan.h>

namespace garlic::clove {
    class VKPipelineObject : public PipelineObject {
        //VARIABLES
    private:
        DevicePointer device;

        VkPipeline pipeline{ VK_NULL_HANDLE };
        VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };

        //FUNCTIONS
    public:
        VKPipelineObject() = delete;
        VKPipelineObject(DevicePointer device, VkPipeline pipeline, VkPipelineLayout pipelineLayout);

        VKPipelineObject(VKPipelineObject const &other) = delete;
        VKPipelineObject(VKPipelineObject &&other) noexcept;

        VKPipelineObject &operator=(VKPipelineObject const &other) = delete;
        VKPipelineObject &operator=(VKPipelineObject &&other) noexcept;

        ~VKPipelineObject();

        VkPipeline getPipeline() const;
        VkPipelineLayout getLayout() const;

        static VkPipelineStageFlags convertStage(Stage garlicStage);
    };
}