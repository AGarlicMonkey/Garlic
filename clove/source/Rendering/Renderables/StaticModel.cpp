#include "Clove/Rendering/Renderables/StaticModel.hpp"

#include "Clove/Rendering/Renderables/Mesh.hpp"

namespace garlic::clove {
    StaticModel::StaticModel(std::vector<std::shared_ptr<Mesh>> meshes, std::shared_ptr<Material> material)
        : meshes(std::move(meshes))
        , material(std::move(material)) {
    }

    StaticModel::StaticModel(StaticModel const &other)
        : material(std::make_shared<Material>(*other.material)) {
        meshes.clear();
        for(auto const &mesh : other.meshes) {
            meshes.emplace_back(std::make_shared<Mesh>(*mesh));
        }
    }

    StaticModel::StaticModel(StaticModel &&other) noexcept = default;

    StaticModel &StaticModel::operator=(StaticModel const &other) {
        material = std::make_shared<Material>(*other.material);
        meshes.clear();
        for(auto const &mesh : other.meshes) {
            meshes.emplace_back(std::make_shared<Mesh>(*mesh));
        }

        return *this;
    }

    StaticModel &StaticModel::operator=(StaticModel &&other) noexcept = default;

    StaticModel::~StaticModel() = default;
}