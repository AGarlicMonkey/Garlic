namespace garlic::clove {
    Application::State Application::getState() const {
        return currentState;
    }
    
    Surface *Application::getSurface() const {
        return surface.get();
    }

    GhaDevice *Application::getGraphicsDevice() const {
        return graphicsDevice.get();
    }

    AhaDevice *Application::getAudioDevice() const {
        return audioDevice.get();
    }

    ForwardRenderer3D *Application::getRenderer() const {
        return renderer.get();
    }

    EntityManager *Application::getEntityManager() const {
        return entityManager.get();
    }

    PhysicsLayer *Application::getPhysicsLayer() const {
        return physicsLayer.get();
    }
}