#pragma once
#include "Core/Math.h"
#include <SDL3/SDL.h>

namespace Vortex {
	class Entity {
	public:
		Vortex::Vec2 position;
		Vortex::Vec2 velocity;
		Vortex::Vec2 acceleration;
		Vortex::Rect collider = {position.x, position.y, 0.0f, 0.0f};

		float mass = 1.0f;
		
		Vortex::Vec2 forceAccumulator = { 0.0f, 0.0f };

		void ApplyForce(Vortex::Vec2 force) {
			forceAccumulator += force;
		}

		void Integrate(float deltaTime) {
			acceleration = forceAccumulator * (1.0f / mass);
			velocity += acceleration * deltaTime;
			position += velocity * deltaTime;
			forceAccumulator = { 0.0f, 0.0f };
		}

		SDL_Texture* sprite = nullptr;
		float width = 128.0f;
		float height = 128.0f;

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
				SDL_FRect destRect = { position.x, position.y, width, height };
				SDL_RenderTexture(renderer, sprite, NULL, &destRect);
			}
		}
	};
}