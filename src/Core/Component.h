#pragma once

#include <string>

namespace Vortex {
    // Forward declaration fixes circular includes.
    class Entity;

    class Component {
    public:
        // Pointer to access owner variables such as position.
        Entity* owner;

        virtual ~Component() {}

        // Called once when attached or when the game starts.
        virtual void Init() {}

        // Called every frame for logic.
        virtual void Update(float deltaTime) {}

        // Called every frame for rendering.
        virtual void Render() {}

        virtual std::string GetName() {
            return "Default";
		}
    };
}
