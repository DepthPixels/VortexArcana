#pragma once
#include "Core/Math.h"
#include <SDL3/SDL.h>

namespace Vortex {
	class Entity {
	public:
		Vortex::Vec2 velocity;
		Vortex::Vec2 acceleration;
		Vortex::Rect bounds = {0.0f, 0.0f, 128.0f, 128.0f};
		float rotation = 0.0f;
		bool isSelected = false;
		bool isBeingDragged = false;
		std::string name = "Default";

		float mass = 1.0f;
		
		Vortex::Vec2 forceAccumulator = { 0.0f, 0.0f };

		void ApplyForce(Vortex::Vec2 force) {
			if (isBeingDragged) return;

			forceAccumulator += force;
		}

		void Integrate(float deltaTime) {
			if (isBeingDragged) return;

			acceleration = forceAccumulator * (1.0f / mass);
			velocity += acceleration * deltaTime;
			bounds.position += velocity * deltaTime;
			forceAccumulator = { 0.0f, 0.0f };
		}

		SDL_Texture* sprite = nullptr;

		void SetSprite(SDL_Renderer* renderer, const char* spriteLocation) {
			SDL_Surface* tempSurface = SDL_LoadBMP(spriteLocation);
			if (!tempSurface) {
				SDL_Log("Failed to load sprite: %s", SDL_GetError());
			}
			else {
				sprite = SDL_CreateTextureFromSurface(renderer, tempSurface);
				if (sprite) {
					SDL_SetTextureScaleMode(sprite, SDL_SCALEMODE_NEAREST);
				}
				SDL_DestroySurface(tempSurface);
			}
		}

		void Draw(SDL_Renderer* renderer) {
			if (sprite) {
				SDL_FRect destRect = { bounds.position.x, bounds.position.y, bounds.w, bounds.h };
				SDL_RenderTexture(renderer, sprite, NULL, &destRect);
				if (isSelected) {
					SDL_SetRenderDrawColor(renderer, 0x45, 0x4A, 0xDE, 0xFF);
					SDL_RenderRect(renderer, &destRect);
				}
			}
		}
	};
}