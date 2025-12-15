#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <string>

#include <glm/glm.hpp>

#include "mcmodel.hpp"
#include "include/f8.hpp"
#include "NonCopyable.hpp"

/// The basis for all visual effect particles and entities
class Particle : public mcmodel::Drawable, NonCopyable {
    private:
        const std::string _uuid;
        bool _dead = false;

    public:
        inline Particle() : _uuid(f8::uuid4()) {};
        inline virtual ~Particle() = default;
        inline virtual void update(const float deltaTime) = 0;

        inline virtual const std::string& getUUID() const noexcept final {
            return this->_uuid;
        }

        /// Marks this particle for removal
        inline virtual void setDead() final {
            this->_dead = true;
        }

        /// True if this particle has been or is about to be removed
        inline virtual bool isDead() const final {
            return this->_dead;
        }

        /// If true, interact() is called with all other particles
        /// Keep this false whenever possible
        inline virtual bool hasInteraction() const {
            return false;
        }

        /// Iff hasInteraction() returns true, called with other interacting
        /// particles so that this particle can update itself or the other
        inline virtual void interact(Particle& other) {}
};

/// Base class for particles that render as a sprite, supports velocity and lifetimes too 
class SpriteParticle : public Particle {
    private:
        double _lifetime = 0.0f;
        glm::vec3 _position;
    protected:
        inline virtual float getSize() const {
            return 0.25f;
        }
        inline virtual float getMaxLifetime() const {
            return 0.0f;
        }
        inline virtual glm::vec3 getTint() const {
            return glm::vec3(1.0f, 1.0f, 1.0f);
        }
        inline virtual glm::vec3 getVelocity() const {
            return glm::vec3(0.0f, 0.0f, 0.0f);
        }
        inline virtual const char getSprite() const = 0;
    public:
        inline SpriteParticle(const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f)) : Particle(), _position(position) {}

        inline virtual double getLifetime() const final {
            return this->_lifetime;
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override final {
            if(ctx.debug()) {
                ctx.getPrimitiveRenderer().point(this->_position, glm::vec3(1.0f, 0.0f, 0.0f));
            }
            
            ctx.getPrimitiveRenderer().sprite(std::string(1, this->getSprite()), this->_position, this->getTint(), this->getSize(), glutils::PrimitiveRenderer::SpriteMode::PARTICLE);
        }

        inline virtual void update(const float deltaTime) override final {
            this->_position += deltaTime*this->getVelocity();
            this->_lifetime += deltaTime;
            if(this->getMaxLifetime() > 0.0f && this->_lifetime >= this->getMaxLifetime()) {
                this->setDead();
            }
        }
};

/// Animated torch smoke particle
class SmokeParticle final : public SpriteParticle {
    private:
        const glm::vec3 _velocity;
    protected:
        inline virtual float getMaxLifetime() const override final {
            return 6.0f;
        }
        inline virtual glm::vec3 getVelocity() const override final {
            return this->_velocity;
        }
        inline virtual glm::vec3 getTint() const override final {
            return glm::vec3(0.2f, 0.2f, 0.2f);
        }
        inline virtual const char getSprite() const override final {
            return '\xe0' + this->getLifetime();
        }
    public:
        inline SmokeParticle(const glm::vec3 position, const float spread = 0.25f) : SpriteParticle(position), _velocity(spread*glm::normalize(glm::vec3(2.0f*f8::randf() - 1.0f, 1.0f, 2.0f*f8::randf() - 1.0f))) {}
};

/// Randomized torch flame particle
class TorchParticle final : public SpriteParticle {
    private:
        const char _variant;
    protected:
        inline virtual float getMaxLifetime() const override final {
            return 2.5f;
        }
        inline virtual const char getSprite() const override final {
            return '\xc4' + this->_variant;
        }
    public:
        inline TorchParticle(const glm::vec3 position) : SpriteParticle(position + 0.0625f*glm::vec3(2.0f*f8::randf() - 1.0f, 2.0f*f8::randf() - 1.0f, 2.0f*f8::randf() - 1.0f)), _variant(f8::randi(0,3)) {}
};

/// Particle used for footsteps
class DustParticle final : public SpriteParticle {
    private:
        const char _variant;
        const glm::vec3 _velocity;
    protected:
        inline virtual float getMaxLifetime() const override final {
            return 1.0f;
        }
        inline virtual const char getSprite() const override final {
            return '\xe0' + this->_variant;
        }
        inline virtual glm::vec3 getVelocity() const override final {
            return this->_velocity;
        }
        inline virtual glm::vec3 getTint() const override final {
            return glm::vec3(0.2f, 0.2f, 0.2f);
        }
    public:
        inline DustParticle(const glm::vec3 position, const float spread) : SpriteParticle(position + spread*glm::vec3(2.0f*f8::randf() - 1.0f, 0.25f*f8::randf(), 2.0f*f8::randf() - 1.0f)), _variant(f8::randi(0,2)), _velocity(0.01f*glm::vec3(2.0f*f8::randf() - 1.0f, 2.0f*f8::randf() - 1.0f, 2.0f*f8::randf() - 1.0f)) {}
};

#endif