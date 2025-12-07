#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "Particle.hpp"
#include "NonCopyable.hpp"
#include "World.hpp"

#include <glm/glm.hpp>

class Entity : public Particle, NonCopyable {
    private:
        bool _hidden = false;
        glm::vec3 _position = {0.0f, 0.0f, 0.0f};
        glm::vec3 _rotation = {0.0f, 0.0f, 0.0f};
    protected:
        const std::shared_ptr<World> _world;
    public:

        inline explicit Entity(const std::shared_ptr<World> world) : _world(world) {
            
        }

        inline virtual ~Entity() = default;

        inline virtual void setPosition(const glm::vec3 position, const bool ignoreCollision = false) {

        }

        inline virtual const glm::vec3 getPosition() const final {
            return this->_position;
        }

        inline virtual void setVelocity(const glm::vec3 position) {

        }

        inline virtual const glm::vec3 getVelocity() const {

        }

        inline virtual bool isTouching(const std::shared_ptr<Entity> other) const {

        }

        inline virtual float getHeight() const = 0;
        inline virtual float getEyeHeight() const = 0;
        inline virtual float getRadius() const = 0;

        inline virtual void setRotation(const glm::vec3 rotation) {

        }

        inline virtual const glm::vec3 getRotation() const final {
            return this->_rotation;
        }

        inline virtual glm::vec3 getForwardVector() const final {

        }

        inline virtual glm::vec3 getUpVector() const final {

        }

        inline virtual void setHidden(const bool hidden = true) final {
            this->_hidden = hidden;
        }

        inline virtual bool isHidden() const final {
            return this->_hidden;
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            if(ctx.debug() || true) {
                ctx.getPrimitiveRenderer().cube(this->getPosition(), {2.0f*this->getRadius(), this->getHeight(), 2.0f*this->getRadius()}, {1.0f, 1.0f, 1.0f}, {true, false, true});
            }
        };
};

#endif