#include "Clove/Graphics/Metal/MetalView.hpp"

@implementation MetalView

+ (Class) layerClass {
	return [CAMetalLayer class];
}

- (instancetype) initWithFrame:(CGRect)frame {
	self = [super initWithFrame:frame];
	if(self != nullptr) {
		self.wantsLayer = YES;
	}
	return self;
}

- (CALayer *) makeBackingLayer {
	_metalLayer = [CAMetalLayer layer];
	[_metalLayer setFramebufferOnly:NO]; //We blit to the layer's drawable so it's technically not a framebuffer only
	_metalLayer.allowsNextDrawableTimeout = NO; //Always blocks until a drawable is ready
	
	return _metalLayer;
}

- (void) setDevice:(id<MTLDevice>)device {
	[_metalLayer setDevice:device];
}

@end
