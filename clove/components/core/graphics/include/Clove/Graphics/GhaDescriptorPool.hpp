#pragma once

#include "Clove/Graphics/Descriptor.hpp"

#include <cinttypes>
#include <memory>
#include <vector>

namespace garlic::clove {
    class GhaDescriptorSet;
    class GhaDescriptorSetLayout;

    /**
     * @brief Information on each type of descriptor allocated from a pool.
     */
    struct DescriptorInfo {
        DescriptorType type;
        uint32_t count; /**< Number of this type to allocate across all DescriptorSets. For example: DescriptorSetBindingInfo::arraySize * GhaDescriptorPool::Descriptor::maxSets. */
    };
}

namespace garlic::clove {
    /**
     * @brief Used to allocate GhaDescriptorSets. GhaDescriptorPool is not tied to a specific GhaDescriptorSetLayout.
     */
    class GhaDescriptorPool {
        //TYPES
    public:
        enum class Flag {
            None,
            FreeDescriptorSet /**< DescriptorSets can be freed individualy. */
        };

        struct Descriptor {
            std::vector<DescriptorInfo> poolTypes;
            Flag flag;
            uint32_t maxSets; /**< The maximum amount of DescriptorSets that can be allocated from this pool. */
        };

        //FUNCTIONS
    public:
        virtual ~GhaDescriptorPool() = default;

        virtual Descriptor const &getDescriptor() const = 0;

        /** 
         * @brief Allocates a descriptor set for each layout provided.
         */
        virtual std::shared_ptr<GhaDescriptorSet> allocateDescriptorSets(std::shared_ptr<GhaDescriptorSetLayout> const &layout)                            = 0;
        virtual std::vector<std::shared_ptr<GhaDescriptorSet>> allocateDescriptorSets(std::vector<std::shared_ptr<GhaDescriptorSetLayout>> const &layouts) = 0;

        /**
         * @brief Free an individual descriptor set. Requires Flag::FreeDescriptorSet to be set on creation.
         */
        virtual void freeDescriptorSets(std::shared_ptr<GhaDescriptorSet> const &descriptorSet) = 0;
        /**
         * @brief Frees individual descriptor sets. Requires Flag::FreeDescriptorSet to be set on creation.
         */
        virtual void freeDescriptorSets(std::vector<std::shared_ptr<GhaDescriptorSet>> const &descriptorSets) = 0;

        /**
         * @brief Resets this pool freeing all DescriptorSets allocated from it.
         */
        virtual void reset() = 0;
    };
}
