#pragma once

#include "Clove/Graphics/Descriptor.hpp"
#include "Clove/Graphics/GhaShader.hpp"

#include <vector>

namespace garlic::clove {
    /**
     * @brief Describes a binding within an individual DescriptorSet.
     */
    struct DescriptorSetBindingInfo {
        uint32_t binding{ 0 };
        DescriptorType type;
        size_t arraySize{ 1 }; /**< If this descriptor represents an array it'll be the size of the array else 1 */
        GhaShader::Stage stage;
    };

    /**
     * @brief Describes the layout of an entire GhaDescriptorSet that will be bound into a pipeline object.
     * @details A 'set' is defined in the shader by: layout(set = 0, binding = 1). This
     * layout describes the type of binding in each binding slot for a given set. The index
     * of the set is defined in the array of DescriptorSetLayouts given to the 
     * PiplineObject::Descriptor.
     * @see GhaDescriptorSet
     * @see GhaDescriptorPool
     */
    class GhaDescriptorSetLayout {
        //TYPES
    public:
        struct Descriptor {
            std::vector<DescriptorSetBindingInfo> bindings{};
        };

        //FUNCTIONS
    public:
        virtual ~GhaDescriptorSetLayout() = default;

        virtual Descriptor const &getDescriptor() const = 0;
    };
}
