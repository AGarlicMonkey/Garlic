#pragma once

#include "Core/Graphics/RenderDevice.hpp"

#include "Graphics/Direct3D/DXGIInfoManager.hpp"

#include <wrl.h>

struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

namespace clv::gfx{
	class Context; //surface
}

namespace clv::gfx::d3d::_11{
	class D3DRenderDevice : public RenderDevice{
		//VARIABLES
	private:
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext;
		
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> defaultRenderTarget;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> currentRenderTarget;

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> defaultDepthStencil;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> currentDepthStencil;

	#if CLV_DEBUG
		static DXGIInfoManager infoManager; //Used by certain DX11 exception macros
	#endif

		math::Vector4f clearColour = { 0.0f, 0.0f, 0.0f, 0.0f };

		//FUNCTIONS
	public:
		D3DRenderDevice() = delete;
		D3DRenderDevice(Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext);
		D3DRenderDevice(const D3DRenderDevice& other) = delete;
		D3DRenderDevice(D3DRenderDevice&& other) noexcept = delete;
		D3DRenderDevice& operator=(const D3DRenderDevice& other) = delete;
		D3DRenderDevice& operator=(D3DRenderDevice&& other) noexcept = delete;
		~D3DRenderDevice();

		virtual void bindIndexBuffer(IndexBuffer& buffer) override;
		virtual void bindVertexBuffer(VertexBuffer& buffer) override;

		virtual void bindShaderResource(ShaderResource& resource) override;

		virtual void bindTexture(Texture& texture) override;

		virtual void bindShader(Shader& shader) override;

		virtual void makeSurfaceCurrent(const Surface& surface) override;

		//Temp: adding default/clear here until I figure out the best way to handle changing the rt for the lights and then back to the surface
		virtual void setRenderTarget(RenderTarget& renderTarget) override;
		virtual void resetRenderTargetToDefault() override;
		//

		virtual void setViewport(const Viewport& viewport) override;

		virtual void clear() override;
		virtual void drawIndexed(const uint32 count) override;

		virtual void setClearColour(const math::Vector4f& colour) override;
		virtual void setDepthBuffer(bool enabled) override;
		virtual void setBlendState(bool enabled) override;

		//TEMP: Need an 'unbind' function or to refactor the bindable system
		virtual void removeCurrentGeometryShader() override;
		virtual void removeTextureAtSlot(uint32 slot) override;
		//

	#if CLV_DEBUG
		static DXGIInfoManager& getInfoManager();
	#endif

	private:
		void setRenderTargetToCurrent();
	};
}