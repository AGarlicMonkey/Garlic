#pragma once

#include "Clove/Graphics/Surface.hpp"

#include <QuartzCore/CAMetalLayer.h>
#include <Metal/Metal.h>
#include <AppKit/AppKit.h> //AppKit for macOS

@interface MTLView : NSView

@property (nonatomic, nonnull, readonly) CAMetalLayer* metalLayer;

- (void) setDevice:(id<MTLDevice>)device;

@end

namespace clv::gfx::mtl{
	class MTLRenderTarget;
}

namespace clv::gfx::mtl{
	class MTLSurface : public Surface{
		//VARIABLES
	private:
		std::shared_ptr<GraphicsFactory> factory;

		MTLView* view = nullptr;
		id<CAMetalDrawable> currentDrawable;
		
		id<MTLTexture> depthTexture;
		
		std::shared_ptr<MTLRenderTarget> renderTarget;

		//FUNCTIONS
	public:
		MTLSurface() = delete;
		MTLSurface(std::shared_ptr<GraphicsFactory> factory, id<MTLDevice> mtlDevice, void* windowData);
		
		MTLSurface(const MTLSurface& other) = delete;
		MTLSurface(MTLSurface&& other) noexcept;

		MTLSurface& operator=(const MTLSurface& other) = delete;
		MTLSurface& operator=(MTLSurface&& other) noexcept;
		
		virtual ~MTLSurface();

		virtual const std::shared_ptr<GraphicsFactory>& getFactory() const override;
		
		virtual void setVSync(bool vsync) override;
		virtual bool isVsync() const override;

		virtual void resizeBuffers(const mth::vec2ui& size) override;

		virtual void makeCurrent() override{}
		
		virtual void present() override;

		virtual std::shared_ptr<RenderTarget> getRenderTarget() const override;
		MTLView* getMTLView() const;
	};
}
