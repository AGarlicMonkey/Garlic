#pragma once

#include <Clove/Application.hpp>
#include <Clove/ECS/Entity.hpp>

namespace garlic::clove {
    class GraphicsImageRenderTarget;
}

namespace garlic::membrane{
    class EditorLayer;
    class RuntimeLayer;
    class ViewportSurface;
}

namespace garlic::membrane {
    /**
     * @brief Translates a Clove session into C++/CLI
     */
public ref class Application {
    private:
        clove::Application *app;
        clove::GraphicsImageRenderTarget *renderTarget;
        ViewportSurface *surface;

        std::shared_ptr<EditorLayer> *editorLayer;
        std::shared_ptr<RuntimeLayer> *runtimeLayer;

        int width;
        int height;

    public:
        Application(int const width, int const height);
        ~Application();
        !Application();

        bool isRunning();
        void tick();
        void render(System::IntPtr backBuffer);
        void shutdown();

        void resize(int width, int height);
    };
}