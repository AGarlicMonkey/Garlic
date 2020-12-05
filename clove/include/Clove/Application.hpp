#pragma once

#include "Clove/LayerStack.hpp"

#include <Clove/Audio/Audio.hpp>
#include <Clove/Graphics/GraphicsAPI.hpp>
#include <Clove/Graphics/GraphicsImage.hpp>
#include <Clove/Platform/PlatformTypes.hpp>
#include <chrono>
#include <optional>
#include <memory>

namespace garlic::clove {
    class Platform;
    class Window;
    class GraphicsDevice;
    class World;
    class Layer;
    class ForwardRenderer3D;
    class GraphicsImageRenderTarget;
    class AudioDevice;
}

namespace garlic::clove {
    class Application {
        //TYPES
    public:
        enum class State {
            Running,
            Stopped
        };

        //VARIABLES
    private:
        static Application *instance;

        std::unique_ptr<GraphicsDevice> graphicsDevice;
        std::unique_ptr<AudioDevice> audioDevice;

        std::unique_ptr<Window> window;
        std::unique_ptr<ForwardRenderer3D> renderer;
        std::unique_ptr<World> world;

        LayerStack layerStack;

        std::chrono::system_clock::time_point prevFrameTime;

        //FUNCTIONS
    public:
        Application(Application const &other)     = delete;
        Application(Application &&other) noexcept = delete;

        Application &operator=(Application const &other) = delete;
        Application &operator=(Application &&other) noexcept = delete;

        ~Application();

        /**
         * @brief Creates a standard Garlic application that opens and manages it's own window
         */
        static std::unique_ptr<Application> create(GraphicsApi graphicsApi, AudioApi audioApi, WindowDescriptor windowDescriptor);

        /**
         * @brief Create a headles Garlic application without a window.
         */
        static std::pair<std::unique_ptr<Application>, GraphicsImageRenderTarget *> createHeadless(GraphicsApi graphicsApi, AudioApi audioApi, GraphicsImage::Descriptor renderTargetDescriptor);

        static Application &get();

        void pushLayer(std::shared_ptr<Layer> layer);

        State getState() const;

        /**
         * @brief Performs a single iteration of the main application loop.
         * @details getState should be called before calling this to check
         * if the application should still be running.
         */
        void tick();

        /**
         * @brief Transition to State::Stoped if the application is in State::Running.
         */
        void shutdown();

        inline GraphicsDevice *getGraphicsDevice() const;
        inline AudioDevice *getAudioDevice() const;

        inline Window *getWindow() const;
        inline ForwardRenderer3D *getRenderer() const;
        inline World *getECSWorld() const;

    private:
        Application();
    };
}

#include "Application.inl"