#pragma once

#include "Clove/ECS/System.hpp"

#include "Clove/ECS/Components/TransformComponent.hpp"
#include "Clove/ECS/Components/Renderable3DComponent.hpp"

namespace clv::gfx{
	class Renderer;
	class Context;
	enum class API;
}

namespace clv::ecs{
	class Render3DSystem : public System<TransformComponent, Renderable3DComponent>{
		//VARIABLES
	public:
		static constexpr SystemID ID = 0x1ae78643; //VS generated GUID

	private:
		std::unique_ptr<gfx::Renderer> renderer;

		//FUNCTIONS
	public:
		Render3DSystem();
		Render3DSystem(const Render3DSystem& other) = delete;
		Render3DSystem(Render3DSystem&& other) noexcept;
		Render3DSystem& operator=(const Render3DSystem& other) = delete;
		Render3DSystem& operator=(Render3DSystem&& other) noexcept;
		virtual ~Render3DSystem();

		virtual void update(float deltaTime) override;

		void initialiseRenderer(const gfx::Context& context);
		gfx::Renderer& getRenderer();

	private:
		math::Matrix4f getTransformWorldMatrix(TransformComponent* component);
	};
}