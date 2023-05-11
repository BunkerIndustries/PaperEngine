#pragma once
#include "_Core.h"
#include "utility.h"

namespace core {

    class Entity;
    class Camera;
    class Layer;

    class Scene {
        friend class Application;
    public:

        Scene();
        virtual ~Scene() = default;

        Shr<Camera> GetCamera();
        glm::vec4 GetBackcolor();

        void Start();
        void Stop();

        void AddEntityToScene(Entity* entity);

        void AddLayer(Layer* layer);
        void AddOverlay(Layer* layer);
        void RemoveLayer(Layer* layer) const;
        void RemoveOverlay(Layer* layer) const;

        std::vector<Entity*>& GetEntitys() { return entities; }

    protected:
        Shr<Camera> camera = nullptr;
        glm::vec3 backcolor = glm::vec3(1.0f);

        virtual void OnStart() = 0;
        virtual void OnStop() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnEvent(Event& e) = 0;
        virtual void Imgui(float dt) {}

    private:
        std::vector<Entity*> entities;
        bool isRunning = false;

        void Update();

    };

}