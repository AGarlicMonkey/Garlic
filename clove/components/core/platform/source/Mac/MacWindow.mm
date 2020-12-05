#include "Clove/Platform/Mac/MacWindow.hpp"

#include "Clove/Graphics/Graphics.hpp"
#include "Clove/Graphics/GraphicsFactory.hpp"
#include "Clove/Graphics/Metal/MTLSurface.hpp"

@implementation MacWindowProxy
- (instancetype)initWithWindowData:(MTLView*)view width: (unsigned int)width height:(unsigned int)height name:(NSString*)name{
	const NSRect rect = NSMakeRect(0, 0, width, height);
	const NSWindowStyleMask styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
	
	_window = [[NSWindow alloc] initWithContentRect:rect
										  styleMask:styleMask
											backing:NSBackingStoreBuffered
											  defer:NO];

	[_window setTitle:name];
	[_window setDelegate:self];
	[_window makeKeyAndOrderFront:nil];

	[_window setContentView:view];
	
	return self;
}

- (void)windowWillClose:(NSNotification *)notification{
	//The application shouldn't shut down when a window closes on MacOS, but this'll do for now
	_cloveWindow->close();
}
@end

namespace garlic::clove{
    MacWindow::MacWindow(const WindowDescriptor& descriptor){
		MacData data = { { descriptor.width, descriptor.height } };
		
		//graphicsFactory = gfx::initialise(descriptor.api);
		//surface = graphicsFactory->createSurface(&data);
		
		NSString* nameString = [NSString stringWithCString:descriptor.title.c_str() encoding:[NSString defaultCStringEncoding]];
		
		windowProxy = [[MacWindowProxy alloc] initWithWindowData:std::static_pointer_cast<gfx::mtl::MTLSurface>(surface)->getMTLView()
														   width:descriptor.width
														  height:descriptor.height
															name:nameString];
		windowProxy.cloveWindow = this;

		open = true;
    }

	MacWindow::~MacWindow(){
		[windowProxy release];
	}
	
	void* MacWindow::getNativeWindow() const{
		return [windowProxy window];
	}
	
	vec2i MacWindow::getPosition() const{
		NSRect frame = [[windowProxy window] frame];
		return { frame.origin.x, frame.origin.y };
	}
	
	vec2i MacWindow::getSize() const{
		NSRect frame = [[windowProxy window] frame];
		return { frame.size.width, frame.size.height };
	}

	void MacWindow::moveWindow(const vec2i& position){
		const vec2i size = getSize();
		[[windowProxy window] setFrame:NSMakeRect(position.x, position.x, size.x, size.y) display:YES];
	}
	
	void MacWindow::resizeWindow(const vec2i& size){
		const vec2i position = getPosition();
		[[windowProxy window] setFrame:NSMakeRect(position.x, position.x, size.x, size.y) display:YES];
	}
	
	bool MacWindow::isOpen() const {
		return open;
	}

	void MacWindow::close() {
		if(onWindowCloseDelegate.isBound()) {
			onWindowCloseDelegate.broadcast();
		}
		open = false;
	}

	void MacWindow::processInput(){
		@autoreleasepool {
			NSEvent* event;
			do{
				event = [NSApp nextEventMatchingMask:NSEventMaskAny
										   untilDate:nil
											  inMode:NSDefaultRunLoopMode
											 dequeue:YES];
			
				vec<2, int32_t, qualifier::defaultp> mouseLoc{ static_cast<int32_t>([NSEvent mouseLocation].x), static_cast<int32_t>([NSEvent mouseLocation].y) };
				switch ([event type]){
					case NSEventTypeKeyDown:
						keyboard.onKeyPressed(static_cast<Key>([event keyCode]));
						break;
					
					case NSEventTypeKeyUp:
						keyboard.onKeyReleased(static_cast<Key>([event keyCode]));
						break;
					
					//TODO: Is there a char or key typed event?
				
					case NSEventTypeMouseEntered:
						mouse.onMouseEnter();
						break;
					
					case NSEventTypeMouseExited:
						mouse.onMouseLeave();
						break;
					
					case NSEventTypeMouseMoved:
						mouse.onMouseMove(mouseLoc.x, mouseLoc.y);
						break;
					
					case NSEventTypeLeftMouseDown:
						mouse.onButtonPressed(MouseButton::_1, mouseLoc.x, mouseLoc.y);
						break;
					
					case NSEventTypeLeftMouseUp:
						mouse.onButtonReleased(MouseButton::_1, mouseLoc.x, mouseLoc.y);
						break;
					
					case NSEventTypeRightMouseDown:
						mouse.onButtonPressed(MouseButton::_2, mouseLoc.x, mouseLoc.y);
						break;
					
					case NSEventTypeRightMouseUp:
						mouse.onButtonReleased(MouseButton::_2, mouseLoc.x, mouseLoc.y);
						break;
					
					case NSEventTypeOtherMouseDown:
						if(([NSEvent pressedMouseButtons] & static_cast<NSUInteger>(MouseButton::_3)) != 0){
							mouse.onButtonPressed(MouseButton::_3, mouseLoc.x, mouseLoc.y);
						}else if(([NSEvent pressedMouseButtons] & static_cast<NSUInteger>(MouseButton::_4)) != 0){
							mouse.onButtonPressed(MouseButton::_4, mouseLoc.x, mouseLoc.y);
						}else if(([NSEvent pressedMouseButtons] & static_cast<NSUInteger>(MouseButton::_5)) != 0){
							mouse.onButtonPressed(MouseButton::_5, mouseLoc.x, mouseLoc.y);
						}
						break;
					
					case NSEventTypeOtherMouseUp:
						if(([NSEvent pressedMouseButtons] & static_cast<NSUInteger>(MouseButton::_3)) != 0){
							mouse.onButtonReleased(MouseButton::_3, mouseLoc.x, mouseLoc.y);
						}else if(([NSEvent pressedMouseButtons] & static_cast<NSUInteger>(MouseButton::_4)) != 0){
							mouse.onButtonReleased(MouseButton::_4, mouseLoc.x, mouseLoc.y);
						}else if(([NSEvent pressedMouseButtons] & static_cast<NSUInteger>(MouseButton::_5)) != 0){
							mouse.onButtonReleased(MouseButton::_5, mouseLoc.x, mouseLoc.y);
						}
						break;
					
					case NSEventTypeScrollWheel:
						mouse.onWheelDelta(static_cast<int32_t>([event scrollingDeltaY]), mouseLoc.x, mouseLoc.y);
						break;
						
					default:
						[NSApp sendEvent:event];
						break;
				}
			} while (event != nil);
		}
	}
}