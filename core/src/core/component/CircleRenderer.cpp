#include "_Core.h"

#include "component/CircleRenderer.h"
#include "generic/Entity.h"
#include "renderer/Renderer.h"


namespace core {

    CircleRenderer::CircleRenderer(glm::vec4 color, float thickness, float fade, bool registerAlphaPixelsToEvent)
	    : color(color), texture(nullptr), thickness(thickness), fade(fade) { }

    CircleRenderer::CircleRenderer(glm::vec4 color, Shr<Texture> texture, float thickness, float fade, bool registerAlphaPixelsToEvent)
        : color(color), texture(texture), thickness(thickness), fade(fade) { }

    void CircleRenderer::OnUpdate() 
    {
        CircleRenderData data;
        data.transform = gameObject->transform;
        data.color = color;
        data.thickness = thickness;
        data.fade = fade;
        data.texture = texture;
        data.mode = gameObject->GetProjectionMode();
        data.coreID = gameObject->GetCoreID();
        data.coreIDToAlphaPixels = registerAlphaPixelsToEvent;
    	Renderer::DrawCircle(data);
    }
}