#pragma once

#include "Bulb/ECS/System.hpp"

namespace clv::plt {
	class Window;
}

namespace blb::rnd {
	class Renderer3D;
}

namespace blb::ecs {
	class RenderSystem : public System {
		//VARIABLES
	private:
		std::unique_ptr<rnd::Renderer3D> renderer;

		//FUNCTIONS
	public:
		RenderSystem() = delete;
		RenderSystem(std::unique_ptr<rnd::Renderer3D> renderer);
		RenderSystem(clv::plt::Window& window);

		RenderSystem(const RenderSystem& other) = delete;
		RenderSystem(RenderSystem&& other) noexcept;

		RenderSystem& operator=(const RenderSystem& other) = delete;
		RenderSystem& operator=(RenderSystem&& other) noexcept;

		~RenderSystem();

		void preUpdate(World& world) override;
		void update(World& world, clv::utl::DeltaTime deltaTime) override;
		void postUpdate(World& world) override;
	};
}