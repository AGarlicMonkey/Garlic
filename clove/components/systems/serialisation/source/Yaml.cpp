#include "Clove/Serialisation/Yaml.hpp"

#include <sstream>
#include <yaml-cpp/yaml.h>

using namespace garlic::clove::serialiser;

namespace garlic::clove {
    namespace {
        void emittNode(YAML::Node &emitterNode, Node const &node) {
            if(node.name.length() <= 0) {
                return;
            }

            if(auto *floatVal{ std::get_if<float>(&node.value) }) {
                emitterNode[node.name] = *floatVal;
            } else if(auto *children{ std::get_if<std::vector<Node>>(&node.value) }; children != nullptr && children->size() > 0) {
                YAML::Node childNode{};
                for(auto &child : *children) {
                    emittNode(childNode, child);
                }
                emitterNode[node.name] = childNode;
            }
        }

        namespace v1 {
            std::string emitt(Node const &rootNode) {
                YAML::Node emitterNode{};

                emitterNode["type"]    = "yaml";
                emitterNode["version"] = 1;

                if(auto *children{ std::get_if<std::vector<Node>>(&rootNode.value) }; children != nullptr) {
                    for(auto &child : *children) {
                        emittNode(emitterNode, child);
                    }
                }

                YAML::Emitter emitter{};
                emitter << emitterNode;

                std::stringstream stream;
                stream << emitter.c_str();

                return stream.str();
            }
        }
    }

    std::string emittYaml(Node const &node){
        return v1::emitt(node);
    }
}