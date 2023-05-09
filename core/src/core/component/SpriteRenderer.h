#pragma once
#include "_Core.h"
#include "utility.h"

#include "generic/Component.h"
#include "event/Event.h"

namespace core {

    enum class Geometry
    {
        NONE,
        RECTANGLE,
        TRIANGLE,
        CIRCLE
    };

    class SpriteRenderer : public RenderComponent {
    public:
        SpriteRenderer(glm::vec4 color, Geometry geometry, bool registerAlphaPixelsToEvent = false);
        SpriteRenderer(glm::vec4 color, Shr<Texture> texture, float tilingFactor, Geometry geometry, bool registerAlphaPixelsToEvent = false);
        

        ~SpriteRenderer() override { }

        void OnStart() override { }
        void OnStop() override { }
        void OnUpdate() override;
        void OnImgui(float dt) override;
        void OnEvent(Event& event) override { }

        void SetColor(glm::vec4 color) { this->color = color; }
        void SetTexture(Shr<Texture> texture) { this->texture = texture; }
        void SetTexCoords(glm::vec2 texCoords[4])
        {
            for (int i = 0; i < 4; i++)
            {
				this->texCoords[i] = texCoords[i];
            }
        }

        glm::vec4 GetColor() const { return this->color; }
        Shr<Texture> GetTexture() { return this->texture; }
        glm::vec2* GetTexCoords() { return this->texCoords; }

    private:
        glm::vec4 color;
        Shr<Texture> texture;
        float tilingFactor;
        glm::vec2 texCoords[4];
        Geometry geometry;
        bool registerAlphaPixelsToEvent;
    };

}