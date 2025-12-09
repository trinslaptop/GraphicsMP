#ifndef MACGUFFIN_HPP
#define MACGUFFIN_HPP

#include <glm/glm.hpp>

#include "Entity.hpp"
#include "World.hpp"
#include "glutils.hpp"
#include "include/f8.hpp"

/// Everyone needs a purpose in life...
/// The player's is chasing this shiny rock
class MacGuffin : public Entity {
    public:
        using Entity::Entity;

        inline virtual float getHeight() const override {
            return 1.0f;
        }

        inline virtual float getEyeHeight() const override {
            return 0.5f;
        }

        inline virtual float getRadius() const override {
            return 0.5f;
        }

        inline virtual int getMaxHealth() const override {
            return 0;
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            Entity::draw(ctx);
            if(!this->isHidden()) {
                ctx.getPrimitiveRenderer().sprite("\x89", this->getPosition() + glm::vec3(0.0f, 0.5f + 0.25f*glm::cos(this->getLifetime()), 0.0f), glutils::hex3(0x00a3f4), 0.5f, glutils::PrimitiveRenderer::SpriteMode::PARTICLE);
            }
        }

        /// Place MacGuffin at random location
        inline void scatter() {
            this->setPosition({f8::randi(4, 4*Chunk::CHUNK_SIZE - 4) + 0.5f, 25.0f, f8::randi(4, 4*Chunk::CHUNK_SIZE - 4) + 0.5f});
            // do {
            // } while(this->getPosition().y != this->getWorld().getTerrainHeight(this->getPosition().x, this->getPosition().z));
        }
};

#endif