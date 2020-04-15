#include "Tunic/UI/Image.hpp"

#include "Tunic/Rendering/Renderables/Sprite.hpp"
#include "Tunic/Rendering/Renderer2D.hpp"

using namespace clv;
using namespace clv::gfx;

namespace tnc::ui {
	Image::Image(std::shared_ptr<GraphicsFactory> graphicsFactory) {
		sprite = std::make_shared<rnd::Sprite>(std::move(graphicsFactory));
	}
	
	void Image::draw(rnd::Renderer2D& renderer, const clv::mth::vec2f& drawSpace) {
		const mth::vec2f screenHalfSize{ static_cast<float>(drawSpace.x) / 2.0f, static_cast<float>(drawSpace.y) / 2.0f };

		const mth::mat4f uiScale = mth::scale(mth::mat4f{ 1.0f }, mth::vec3f{ 40.0f });

		//Move the position to origin at the top left
		const mth::vec2f pos = { position.x - screenHalfSize.x, -position.y + screenHalfSize.y };

		const mth::mat4f translation	= mth::translate(mth::mat4f{ 1 }, { pos, 0.0f });
		const mth::mat4f rotation		= mth::rotate(mth::mat4f{ 1 }, this->rotation, { 0.0f, 0.0f, 1.0f });
		const mth::mat4f scale			= mth::scale(mth::mat4f{ 1 }, { this->scale, 0.0f });

		const mth::mat4f model = translation * rotation * scale;
		const mth::mat4f projection = mth::createOrthographicMatrix(-screenHalfSize.x, screenHalfSize.x, -screenHalfSize.y, screenHalfSize.y);

		sprite->getMaterialInstance().setData(BufferBindingPoint::BBP_2DData, projection * model * uiScale, ShaderStage::Vertex);

		renderer.submitWidget(sprite);
	}
}