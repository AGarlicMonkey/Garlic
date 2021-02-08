#include "Clove/Application.hpp"

#include "Clove/InputEvent.hpp"
#include "Clove/Layer.hpp"
#include "Clove/Layers/AudioLayer.hpp"
#include "Clove/Layers/PhysicsLayer.hpp"
#include "Clove/Layers/RenderLayer.hpp"
#include "Clove/Layers/TransformLayer.hpp"
#include "Clove/Rendering/ForwardRenderer3D.hpp"
#include "Clove/Rendering/GraphicsImageRenderTarget.hpp"
#include "Clove/Rendering/SwapchainRenderTarget.hpp"
#include "Clove/WindowSurface.hpp"

#include <Clove/Audio/AhaDevice.hpp>
#include <Clove/Definitions.hpp>
#include <Clove/ECS/EntityManager.hpp>
#include <Clove/Graphics/GhaDevice.hpp>
#include <Clove/Graphics/Graphics.hpp>
#include <Clove/Log/Log.hpp>
#include <Clove/Platform/Platform.hpp>
#include <Clove/Platform/Window.hpp>

namespace garlic::clove {
    Application *Application::instance{ nullptr };

    Application::~Application() = default;

    std::unique_ptr<Application> Application::create(GraphicsApi graphicsApi, AudioApi audioApi, WindowDescriptor const &windowDescriptor) {
        //If we're managing our own window then we need to do any platform initialisation
		Platform::initialise();
		
		auto window{ Platform::createWindow(windowDescriptor) };
		auto *windowPtr{ window.get() };
		
		auto graphicsDevice{ createGraphicsDevice(graphicsApi, window->getNativeWindow()) };
		auto audioDevice{ createAudioDevice(audioApi) };
		
		auto surface{ std::make_unique<WindowSurface>(std::move(window)) };
		
		auto renderTarget{ std::make_unique<SwapchainRenderTarget>(*surface, graphicsDevice.get()) };
		
		std::unique_ptr<Application> app{ new Application{ std::move(graphicsDevice), std::move(audioDevice), std::move(surface), std::move(renderTarget) } };
		windowPtr->onWindowCloseDelegate.bind(&Application::shutdown, app.get());
		
        return app;
    }

    std::pair<std::unique_ptr<Application>, GraphicsImageRenderTarget *> Application::createHeadless(GraphicsApi graphicsApi, AudioApi audioApi, GhaImage::Descriptor renderTargetDescriptor, std::unique_ptr<Surface> surface) {
		auto graphicsDevice{ createGraphicsDevice(graphicsApi, std::any{}) };
		auto audioDevice{ createAudioDevice(audioApi) };
		
		auto renderTarget{ std::make_unique<GraphicsImageRenderTarget>(renderTargetDescriptor, graphicsDevice->getGraphicsFactory()) };
		auto *renderTargetPtr{ renderTarget.get() };
		
		std::unique_ptr<Application> app{ new Application{ std::move(graphicsDevice), std::move(audioDevice), std::move(surface), std::move(renderTarget) } };

        return { std::move(app), renderTargetPtr };
    }

    Application &Application::get() {
        return *instance;
    }

    void Application::pushLayer(std::shared_ptr<Layer> layer, LayerGroup group) {
        CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Trace, "Attached layer: {0}", layer->getName());
        layer->onAttach();
        layers[group].push_back(std::move(layer));
    }

    void Application::popLayer(std::shared_ptr<Layer> const &layer) {
        for(auto &&[key, group] : layers) {
            if(auto it = std::find(group.begin(), group.end(), layer); it != group.end()) {
                CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Trace, "Popped layer: {0}", (*it)->getName());
                (*it)->onDetach();
                group.erase(it);
                break;
            }
        }
    }

    void Application::tick() {
        //Process input first incase we get a close event.
        surface->processInput();

        if(currentState != State::Running) {
            return;
        }

        auto const currFrameTime{ std::chrono::system_clock::now() };
        std::chrono::duration<float> const deltaSeonds{ currFrameTime - prevFrameTime };
        prevFrameTime = currFrameTime;

        renderer->begin();

        while(auto keyEvent = surface->getKeyboard().getKeyEvent()) {
            InputEvent const event{ *keyEvent, InputEvent::Type::Keyboard };
            for(auto &&[key, group] : layers) {
                for(auto &layer : group) {
                    if(layer->onInputEvent(event) == InputResponse::Consumed) {
                        break;
                    }
                }
            }
        }
        while(auto mouseEvent = surface->getMouse().getEvent()) {
            InputEvent const event{ *mouseEvent, InputEvent::Type::Mouse };
            for(auto &&[key, group] : layers) {
                for(auto &layer : group) {
                    if(layer->onInputEvent(event) == InputResponse::Consumed) {
                        break;
                    }
                }
            }
        }

        for(auto &&[key, group] : layers) {
            for(auto &layer : group) {
                layer->onUpdate(deltaSeonds.count());
            }
        }

        renderer->end();
    }

    void Application::shutdown() {
        currentState = State::Stopped;
        graphicsDevice->waitForIdleDevice();
        surface.reset();
    }

    Application::Application(std::unique_ptr<GhaDevice> graphicsDevice, std::unique_ptr<AhaDevice> audioDevice, std::unique_ptr<Surface> surface, std::unique_ptr<RenderTarget> renderTarget)
		: graphicsDevice{ std::move(graphicsDevice) }
		, audioDevice{ std::move(audioDevice) }
		, surface{ std::move(surface) }{
        CLOVE_ASSERT(instance == nullptr, "Only one Application can be active");
        instance = this;

        prevFrameTime = std::chrono::system_clock::now();
		
		//Systems
		renderer = std::make_unique<ForwardRenderer3D>(this->graphicsDevice.get(), std::move(renderTarget));
		entityManager = std::make_unique<EntityManager>();
		
		//Layers
		physicsLayer = std::make_shared<PhysicsLayer>(entityManager.get());
		
		pushLayer(std::make_shared<TransformLayer>(entityManager.get()), LayerGroup::Initialisation);
		pushLayer(physicsLayer, LayerGroup::Initialisation);
		pushLayer(std::make_shared<AudioLayer>(entityManager.get()), LayerGroup::Render);
		pushLayer(std::make_shared<RenderLayer>(renderer.get(), entityManager.get()), LayerGroup::Render);
    }
}
