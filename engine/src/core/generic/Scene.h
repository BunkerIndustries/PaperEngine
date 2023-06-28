#pragma once
#include "Engine.h"
#include "utility.h"

#include "utils/UUID.h"

#include "generic/EditorCamera.h"

namespace ppr {

    class Entity;

    class Scene {
        friend class Application;
        friend class YAMLSerializer;
    public:

        Scene();
        Scene(const UUID& uuid);
        Scene(const std::string& name);
        Scene(const UUID& uuid, const std::string& name);
        ~Scene();

        void Start();

        void Stop();

        void Render(const Shr<EditorCamera>& camera);

        Entity CreateEntity(const std::string& name);
        Entity CreateEntity(const UUID& id, const std::string& name);
        bool DestroyEntity(Entity entity);

        Entity GetEntity(const UUID& id);
        UUID GetUUID() const { return uuid; }
        std::string GetName() const { return name; }

        std::filesystem::path GetPath() const { return path; }
        void SetPath(const std::filesystem::path& path ) { this->path = path; }

        auto& Registry() { return registry; }
        auto& EntityMap() { return entity_map; }

        bool IsDirty() const { return is_dirty; }
        void SetClean() { is_dirty = false; }

    private:
        UUID uuid;
        std::string name;

        bool is_dirty = false;

        std::filesystem::path path;

        entt::registry registry;
        std::unordered_map<UUID, entt::entity> entity_map;
    };

}