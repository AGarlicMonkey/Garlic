#pragma once

#include "Clove/Graphics/GhaDescriptorPool.hpp"

#include <MetalKit/MetalKit.h>

namespace garlic::clove {
	class MetalDescriptorPool : public GhaDescriptorPool {
		//VARIABLES
	private:
		Descriptor descriptor{};
		
		id<MTLDevice> device{ nullptr };
		
		//FUNCTIONS
	public:
		MetalDescriptorPool() = delete;
		MetalDescriptorPool(Descriptor descriptor, id<MTLDevice> device);
		
		MetalDescriptorPool(MetalDescriptorPool const &other) = delete;
		MetalDescriptorPool(MetalDescriptorPool &&other) noexcept;
		
		MetalDescriptorPool &operator=(MetalDescriptorPool const &other) = delete;
		MetalDescriptorPool &operator=(MetalDescriptorPool &&other) noexcept;
		
		~MetalDescriptorPool();
		
		Descriptor const &getDescriptor() const override;

		std::shared_ptr<GhaDescriptorSet> allocateDescriptorSets(std::shared_ptr<GhaDescriptorSetLayout> const &layout) override;
		std::vector<std::shared_ptr<GhaDescriptorSet>> allocateDescriptorSets(std::vector<std::shared_ptr<GhaDescriptorSetLayout>> const &layouts) override;

		void freeDescriptorSets(std::shared_ptr<GhaDescriptorSet> const &descriptorSet) override;
		void freeDescriptorSets(std::vector<std::shared_ptr<GhaDescriptorSet>> const &descriptorSets) override;

		void reset() override;
	};
}
