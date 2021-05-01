#include "Membrane/Application.hpp"

#include "Membrane/EditorLayer.hpp"
#include "Membrane/RuntimeLayer.hpp"
#include "Membrane/ViewportSurface.hpp"

#include <Clove/Application.hpp>
#include <Clove/Audio/Audio.hpp>
#include <Clove/Components/StaticModelComponent.hpp>
#include <Clove/Components/TransformComponent.hpp>
#include <Clove/ECS/EntityManager.hpp>
#include <Clove/Graphics/GraphicsAPI.hpp>
#include <Clove/Graphics/GhaBuffer.hpp>
#include <Clove/Graphics/GhaImage.hpp>
#include <Clove/Log/Log.hpp>
#include <Clove/Rendering/GraphicsImageRenderTarget.hpp>

namespace garlic::membrane {
    Application::Application(int const width, int const height)
        : width{ width }
        , height{ height } {
        using namespace garlic::clove;

        GhaImage::Descriptor renderTargetImageDescriptor{};
        renderTargetImageDescriptor.type        = GhaImage::Type::_2D;
        renderTargetImageDescriptor.usageFlags  = GhaImage::UsageMode::ColourAttachment | GhaImage::UsageMode::TransferSource;
        renderTargetImageDescriptor.dimensions  = { width, height };
        renderTargetImageDescriptor.format      = GhaImage::Format::B8G8R8A8_SRGB;//Hard coding format to B8G8R8A8_SRGB as that is what the WriteableBitmap is set to
        renderTargetImageDescriptor.sharingMode = SharingMode::Concurrent;

        //Use pair as there seems to be an issue when using structured bindings
        auto vpSurface{ std::make_unique<ViewportSurface>() };
        surface = vpSurface.get();
        auto pair{ clove::Application::createHeadless(GraphicsApi::Vulkan, AudioApi::OpenAl, std::move(renderTargetImageDescriptor), std::move(vpSurface)) };
        app          = pair.first.release();
        renderTarget = pair.second;

        editorLayer  = new std::shared_ptr<EditorLayer>();
        *editorLayer = std::make_shared<EditorLayer>();

        runtimeLayer  = new std::shared_ptr<RuntimeLayer>();
        *runtimeLayer = std::make_shared<RuntimeLayer>();

        app->pushLayer(*runtimeLayer);
        app->pushLayer(*editorLayer);
    }

    Application::~Application() {
        this->!Application();
    }

    Application::!Application() {
        delete app;
        delete editorLayer;
        delete runtimeLayer;
    }

    bool Application::isRunning() {
        return app->getState() != clove::Application::State::Stopped;
    }

    void Application::tick() {
        app->tick();
    }

    void Application::render(System::IntPtr backBuffer) {
        auto const renderTargetBuffer{ renderTarget->getNextReadyBuffer() };
        size_t constexpr bbp{ 4 };
        renderTargetBuffer->read(backBuffer.ToPointer(), 0, width * height * bbp);
    }

    void Application::shutdown() {
        app->shutdown();
    }

    void Application::resize(int width, int height) {
        clove::vec2i const size{ width, height };
        renderTarget->resize(size);
        surface->setSize(std::move(size));

        this->width  = width;
        this->height = height;
    }
}