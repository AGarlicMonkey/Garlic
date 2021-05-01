#pragma once

#include <Clove/SerialisationCommon.hpp>

namespace garlic::membrane {
    /**
     * @brief Holds the name of the entity used in the editor
     */
    struct NameComponent {
        std::string name;
    };
}

namespace garlic::clove {
    template<>
    inline serialiser::Node serialise(membrane::NameComponent const &object) {
        serialiser::Node node{};
        node["name"] = object.name;
        return node;
    }

    template<>
    inline membrane::NameComponent deserialise(serialiser::Node const &node) {
        membrane::NameComponent component{};
        component.name = node["name"].as<std::string>();
        return component;
    }
}
