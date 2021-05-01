#pragma once

#include <Clove/Platform/Input/Keyboard.hpp>
#include <Clove/Platform/Input/Mouse.hpp>
#include <Clove/Surface.hpp>
#include <msclr/gcroot.h>
#include <vector>

namespace garlic::membrane {
    /**
     * @brief A surface backed by an editor viewport.
     */
    class ViewportSurface : public clove::Surface {
        friend ref class ViewportSurfaceMessageProxy;

        //TYPES
    private:
        /**
         * @brief Holds any input event recieved.
         */
        struct GenericEvent{
            enum class Type{
                Keyboard,
                Mouse
            };
            enum class State{
                None,
                Pressed,
                Released,
            };

            Type type;
            State state;

            clove::Key key{ clove::Key::None };

            clove::MouseButton button{ clove::MouseButton::None };
            clove::vec2i pos{};
        };

        //VARIABLES
    private:
        msclr::gcroot<ViewportSurfaceMessageProxy ^> messageProxy;

        clove::MultiCastDelegate<void(clove::vec2ui const &)> resizeDelegate{};

        clove::vec2i size{};

        std::vector<GenericEvent> events;

        clove::Keyboard keyboard;
        clove::Mouse mouse;

        clove::Keyboard::Dispatcher keyboardDispatcher{};
        clove::Mouse::Dispatcher mouseDispatcher{};

        //FUNCTIONS
    public:
        //TODO: Ctors
        ViewportSurface();

        clove::MultiCastDelegate<void(clove::vec2ui const &)> &onSurfaceResize() override;

        void processInput() override;

        void setSize(clove::vec2i size);
        clove::vec2i getSize() const override;

        clove::Keyboard &getKeyboard() override;
        clove::Mouse &getMouse() override;
    };
}