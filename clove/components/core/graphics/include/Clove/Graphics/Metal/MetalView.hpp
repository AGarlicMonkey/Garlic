#pragma once

#include <MetalKit/MetalKit.h>

/**
 @brief Custom view implementation. Allows a window's view to be backed by a metal layer.
 */
@interface MetalView : NSView

@property (nonatomic, nonnull, readonly) CAMetalLayer* metalLayer;

+ (Class) layerClass;

- (CALayer *) makeBackingLayer;
- (void) setDevice:(id<MTLDevice>)device;

@end
