#pragma once

#include "Clove/Graphics/Descriptor.hpp"
#include "Clove/Graphics/GhaImage.hpp"

#include <span>

namespace garlic::clove {
    class GhaBuffer;
    class GhaSampler;
    class GhaImageView;
}

namespace garlic::clove {
    /**
     * @brief Used to associate data from certain containers into a binding inside a GhaShader.
     * @details A 'set' is defined in the shader by: layout(set = 0, binding = 1). Whenever
     * this set is bound it will represent what ever number is passed to 
     * GhaGraphicsCommandBuffer::bindDescriptorSet which has to match the index of the 
     * GhaDescriptorSetLayout in the array inside PiplineObject::Descriptor this set was allocated
     * from. Each binding is written to through this GhaDescriptorSet
     * @note Once a container has been written to a GhaDescriptorSet then when ever the
     * GhaDescriptorSet is bound then that container's data will be used for the associated
     * binding slot. This allows multiple sets to reuse the same data.
     * @see GhaDescriptorSetLayout
     * @see GhaDescriptorPool
     */
    class GhaDescriptorSet {
        //FUNCTIONS
    public:
        virtual ~GhaDescriptorSet() = default;

        /**
         * @brief Maps a region of a buffer into a binding inside a shader.
         * @param buffer The buffer to map from.
         * @param offset An offset into the buffer to start mapping from.
         * @param range The size of the region into the buffer to map.
         * @param bindingSlot The binding slot inside the shader to map to.
         */
        virtual void map(GhaBuffer const &buffer, size_t const offset, size_t const range, DescriptorType const descriptorType, uint32_t const bindingSlot) = 0;

        /**
         * @brief Maps an image view into a binding inside a shader.
         * @param imageView The image view to map.
         * @param layout The layout of the image being mapped.
         * @param bindingSlot The binding slot inside the shader to map to.
         */
        virtual void map(GhaImageView const &imageView, GhaImage::Layout const layout, uint32_t const bindingSlot) = 0;

        /**
         * @brief Maps a sampler into a binding inside a shader.
         * @param sampler The sampler to map.
         * @param bindingSlot The binding slot inside the shader to map to.
         */
        virtual void map(GhaSampler const &sampler, uint32_t const bindingSlot) = 0;
    };
}