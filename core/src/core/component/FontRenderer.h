#pragma once
#include "_Core.h"

#include "generic/GameObject_Component.h"

namespace core {

    class FontRenderer : public Component {
    private:

    public:
        FontRenderer();
        ~FontRenderer();
        void start() override;
        void update(float deltaTime) override;
    };

}