#pragma once

#include <Clove/Audio/Audio.hpp>
#include <Clove/Graphics/GraphicsAPI.hpp>
#include <Clove/Graphics/GhaImage.hpp>
#include <Clove/Platform/PlatformTypes.hpp>
#include <chrono>
#include <map>
#include <memory>
#include <vector>

namespace garlic::clove {
    class Platform;
    class Surface;
    class GhaDevice;
    class EntityManager;
    class Layer;
    class ForwardRenderer3D;
    class GraphicsImageRenderTarget;
    class AhaDevice;
    class PhysicsLayer;
	class RenderTarget;
}

namespace garlic::clove {
    class Application {
        //TYPES
    public:
        enum class State {
            Running,
            Stopped
        };

        enum class LayerGroup {
            Initialisation, /**< For layers that need to perform logic before anything else. */
            Core,           /**< The default group. For layers that contain core / general functionality. */
            Interface,      /**< For layers that contain interface logic. Usually UI */
            Render,         /**< For layers that need be run very last. Usually for some form of rendering logic. */
        };

        //VARIABLES
    private:
        static Application *instance;
        State currentState{ State::Running }; //Assumed to be initialised to the running state.

        std::unique_ptr<GhaDevice> graphicsDevice;
        std::unique_ptr<AhaDevice> audioDevice;
		
		std::unique_ptr<Surface> surface;

        std::unique_ptr<ForwardRenderer3D> renderer;
        std::unique_ptr<EntityManager> entityManager;
        std::shared_ptr<PhysicsLayer> physicsLayer;

        std::map<LayerGroup, std::vector<std::shared_ptr<Layer>>> layers;

        std::chrono::system_clock::time_point prevFrameTime;

        //FUNCTIONS
    public:
		Application() = delete;
		
        Application(Application const &other)     = delete;
        Application(Application &&other) noexcept = delete;

        Application &operator=(Application const &other) = delete;
        Application &operator=(Application &&other) noexcept = delete;

        ~Application();

        /**
         * @brief Creates a standard Garlic application that opens and manages it's own window.
         * @param graphicsApi Which graphics api to use.
         * @param audioApi Which audio api to use.
         * @param windowDescriptor A descriptor describing the properties of the window.
         * @return The created application instance.
         */
        static std::unique_ptr<Application> create(GraphicsApi graphicsApi, AudioApi audioApi, WindowDescriptor const &windowDescriptor);

        /**
         * @brief Create a Garlic application without a window that is capable of offscreen rendering.
         * @param graphicsApi Which graphics api to use.
         * @param audioApi Which audio api to use.
         * @param renderTargetDescriptor A descriptor describing the format of the target that'll be rendered to.
         * @param surface A surface to provide input functionality to the application.
         * @return A pair with the created application instance and a pointer to the render target of the application.
         */
        static std::pair<std::unique_ptr<Application>, GraphicsImageRenderTarget *> createHeadless(GraphicsApi graphicsApi, AudioApi audioApi, GhaImage::Descriptor renderTargetDescriptor, std::unique_ptr<Surface> surface);

        static Application &get();

        void pushLayer(std::shared_ptr<Layer> layer, LayerGroup group = LayerGroup::Core);
        void popLayer(std::shared_ptr<Layer> const &layer);

        inline State getState() const;

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

        inline Surface *getSurface() const;

        //Devices
        inline GhaDevice *getGraphicsDevice() const;
        inline AhaDevice *getAudioDevice() const;

        //Systems
        inline ForwardRenderer3D *getRenderer() const;
        inline EntityManager *getEntityManager() const;
        inline PhysicsLayer *getPhysicsLayer() const;

    private:
        Application(std::unique_ptr<GhaDevice> graphicsDevice, std::unique_ptr<AhaDevice> audioDevice, std::unique_ptr<Surface> surface, std::unique_ptr<RenderTarget> renderTarget);
    };
}

#include "Application.inl"
