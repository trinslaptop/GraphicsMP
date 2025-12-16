#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "Particle.hpp"
#include "NonCopyable.hpp"
#include "World.hpp"
#include "Block.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <memory>

/// Axis-Aligned Bounding Box
struct AABB {
    const glm::vec3 min, max;

    inline static const glm::vec3 getCenter(const AABB& a) {
        return (a.min + a.max)/2.0f;
    }

    inline static const glm::vec3 getSize(const AABB& a) {
        return a.max - a.min;
    }

    /// Gets the amount of overlap along each axis, directionless
    inline static const glm::vec3 getOverlap(const AABB& a, const AABB& b) {
        return glm::min(a.max, b.max) - glm::max(a.min, b.min);
    }

    /// Gets the minumum translation vector needed to rectify collision
    inline static const glm::vec3 getMTV(const AABB&a, const AABB& b) {
        const glm::vec3 overlap = AABB::getOverlap(a, b), ca = AABB::getCenter(a), cb = AABB::getCenter(b);
        const glm::vec3 translation = (1.0f - 2.0f*glm::vec3(glm::lessThan(AABB::getCenter(a), AABB::getCenter(b))))*overlap;

        // Find minimum axis, prefer y
        if(overlap.y <= overlap.x && overlap.y <= overlap.z) {
            return glm::vec3(0.0f, translation.y, 0.0f);
        } else if (overlap.x <= overlap.z) {
            return glm::vec3(translation.x, 0.0f, 0.0f);
        } else {
            return glm::vec3(0.0f, 0.0f, translation.z);
        }
    }
};

/// Entities are more advanced particles with gameplay features like AABB collision
class Entity : public Particle {
    private:
        bool _hidden = false, _invulnerable = false;
        glm::vec3 _position = {0.0f, 0.0f, 0.0f};
        glm::vec3 _last_position = {0.0f, 0.0f, 0.0f};
        glm::vec3 _rotation = {0.0f, 0.0f, 0.0f};
        double _lifetime = 0.0f;
        int _health = 0;
        float _hurttime = 0;

        float _limbSwingAmount = 0.0, _prevLimbSwingAmount = 0.0, _limbSwing = 0.0;

        World& _world;
    public:
        /// During this timeframe after taking damage, entities are temporarily immune to additional damage
        static constexpr float HURT_DURATION = 1.5f;

        inline explicit Entity(World& world) : _world(world) {}

        inline virtual ~Entity() = default;

        inline virtual int getMaxHealth() const = 0;
        inline virtual float getHeight() const = 0;
        inline virtual float getEyeHeight() const = 0;
        inline virtual float getRadius() const = 0;

        inline virtual const glm::vec3 getVelocity() const {
            return glm::vec3(0.0f, 0.0f, 0.0f);
        };

        inline virtual float getGravity() const {
            return 9.8;
        }

        inline virtual void setPosition(const glm::vec3 position) final {
            // Clamp to terrain if in valid chunk
            this->_position = glm::vec3(position.x, glm::max(position.y, this->getWorld().getTerrainHeight(position.x, position.z)), position.z);
        
            // Fix block collision
            const glm::ivec3 imin = glm::floor(this->getAABB().min), imax = glm::ceil(this->getAABB().max);
            for(int x = imin.x; x < imax.x; x++) {
                for(int y = imin.y; y < imax.y; y++) {
                    for(int z = imin.z; z < imax.z; z++) {
                        const std::shared_ptr<Block> block = this->getWorld().getBlock(glm::ivec3(x, y, z));
                        if(block && block->isSolid()) {
                            this->_position += this->getMTV(AABB {
                                .min = glm::vec3(x,y,z),
                                .max = glm::vec3(x + 1.0f, y + 1.0f, z + 1.0f)
                            });
                        }
                    }
                }
            }
        }

        inline virtual const glm::vec3 getPosition() const final {
            return this->_position;
        }

        inline virtual const glm::vec3 getLastPosition() const final {
            return this->_last_position;
        }

        inline virtual void setRotation(const glm::vec3 rotation) final {
            this->_rotation = glm::mod(rotation, 2.0f*glutils::PI);
        }

        inline virtual const glm::vec3 getRotation() const final {
            return this->_rotation;
        }


        inline virtual int getHealth() const final {
            return this->_health;
        }

        inline virtual float getHurtTime() const final {
            return this->_hurttime;
        }

        /// Directly modifies the entity health
        /// NOTE: If you're trying to deal damage to the entity use damage() instead
        inline virtual void setHealth(const int health) final {
            this->_health = glm::clamp(health, 0, this->getMaxHealth());
        }

        inline virtual void setInvulnerable(const bool invulnerable = true) final {
            this->_invulnerable = invulnerable;
        }

        inline virtual bool isInvulnerable() const final {
            return this->_invulnerable;
        }

        /// Deals damage to the entity, if absolute is true ignores invulnerability
        inline virtual void damage(const int amount = 1, bool absolute = false) final {
            if((!this->isInvulnerable() || absolute) && this->_hurttime == 0.0f && this->getHealth() > 0) {
                this->setHealth(this->getHealth() - amount);
                this->_hurttime = Entity::HURT_DURATION;
            }
        }

        inline virtual const AABB getAABB() const final {
            return AABB {
                .min = this->getPosition() - glm::vec3(this->getRadius(), 0.0f, this->getRadius()),
                .max = this->getPosition() + glm::vec3(this->getRadius(), this->getHeight(), this->getRadius())
            };
        }

        /// Gets the amount of overlap along each axis, directionless
        inline virtual const glm::vec3 getOverlap(const AABB& other) const final {
            return AABB::getOverlap(this->getAABB(), other);
        }

        /// Gets the amount of overlap along each axis, directionless
        inline virtual const glm::vec3 getOverlap(const Entity& other) const final {
            return AABB::getOverlap(this->getAABB(), other.getAABB());
        }

        /// Gets the minumum translation vector needed to rectify collision
        inline virtual const glm::vec3 getMTV(const AABB& other) const final {
            return AABB::getMTV(this->getAABB(), other);
        }

        /// Gets the minumum translation vector needed to rectify collision
        inline virtual const glm::vec3 getMTV(const Entity& other) const final {
            return AABB::getMTV(this->getAABB(), other.getAABB());
        }

        /// Returns true if two entities touch
        inline virtual bool isTouching(const Entity& other) const final {
            return glm::all(glm::greaterThan(this->getOverlap(other), glm::vec3(0.0f, 0.0f, 0.0f)));
        }

        /// Gets the horizontal forward vector (not look vector)
        inline virtual glm::vec3 getForwardVector() const final {
            return glm::vec3(glm::yawPitchRoll(this->getRotation().x, this->getRotation().y, this->getRotation().z)*glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        }

        inline virtual glm::vec3 getUpVector() const final {
            return glm::vec3(glm::yawPitchRoll(this->getRotation().x, this->getRotation().y, this->getRotation().z)*glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
        }

        inline virtual glm::vec3 getEyePosition() const final {
            return this->getPosition() + glm::vec3(0.0f, this->getEyeHeight(), 0.0f);
        }

        inline virtual void setHidden(const bool hidden = true) final {
            this->_hidden = hidden;
        }

        inline virtual bool isHidden() const final {
            return this->_hidden;
        }

        /// Gets the total time this entity has been alive
        inline virtual double getLifetime() const final {
            return this->_lifetime;
        }

        inline virtual World& getWorld() const final {
            return this->_world;
        }

        inline virtual float getLimbSwing(const float deltaTime) const final {
            return this->_limbSwing - this->_limbSwingAmount*(1.0f - deltaTime);
        }

        inline virtual float getLimbSwingAmount(const float deltaTime) const final {
            return this->_prevLimbSwingAmount + deltaTime*(this->_limbSwingAmount - this->_prevLimbSwingAmount);
        }

        /// True if the entity is in the air or a non-solid block
        inline virtual bool isInAir() const final {
            const std::shared_ptr<Block> block = this->getWorld().getBlock(this->getPosition() - glm::vec3(0.0f, 1.0f, 0.0f));
            return (!block || !block->isSolid()) && this->getPosition().y != this->getWorld().getTerrainHeight(this->getPosition().x, this->getPosition().z);
        } 

        /// Draws debug info, make sure to call this if overriding
        inline virtual void draw(glutils::RenderContext& ctx) const override {
            if(ctx.debug()) {
                ctx.getPrimitiveRenderer().sprite(std::to_string(this->getHealth()), this->getPosition() + glm::vec3(0.0f, this->getHeight() + 0.25, 0.0f), {1.0f, 0.0f, 0.0f}, 0.5, glutils::PrimitiveRenderer::SpriteMode::PARTICLE);
                ctx.getPrimitiveRenderer().line(this->getEyePosition(), this->getEyePosition() + this->getForwardVector(), {0.0f, 0.0f, 1.0f});
                
                ctx.getPrimitiveRenderer().cube(glm::ivec3(this->getPosition() - glm::vec3(0.0f, 1.0f, 0.0f)), glm::ivec3(1, 1, 1), this->isInAir() ? glm::vec3(0.0f, 1.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 1.0f));

                // Render collision info
                const AABB aabb = this->getAABB();
                ctx.getPrimitiveRenderer().cube(aabb.min, AABB::getSize(aabb), {1.0f, 1.0f, 1.0f});
                const glm::ivec3 imin = glm::floor(aabb.min), imax = glm::ceil(aabb.max);
                for(int x = imin.x; x < imax.x; x++) {
                    for(int y = imin.y; y < imax.y; y++) {
                        for(int z = imin.z; z < imax.z; z++) {
                            const std::shared_ptr<Block> block = this->getWorld().getBlock(glm::ivec3(x, y, z));
                            ctx.getPrimitiveRenderer().cube({x,y,z}, {1,1,1}, block && block->isSolid() ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f));
                        }
                    }
                }
            }
        }

        /// Handles base entity logic, make sure to call this if overriding
        inline virtual void update(const float deltaTime) override {
            this->_lifetime += deltaTime;
            this->_hurttime = glm::max(0.0f, this->_hurttime - deltaTime);

            this->_prevLimbSwingAmount = this->_limbSwingAmount;
            this->_limbSwingAmount += (glm::min(1.0f, glm::length(glm::vec2(this->getPosition().x, this->getPosition().z) - glm::vec2(this->getLastPosition().x, this->getLastPosition().z))*2.0f) - this->_limbSwingAmount)*0.5f;
            this->_limbSwing += this->_limbSwingAmount;

            this->_last_position = this->_position;

            this->setPosition(this->getPosition() + deltaTime*glm::vec3(glm::vec4(this->getVelocity(), 0.0f)*glm::rotate(glm::mat4(1.0f), -this->getRotation().x, this->getUpVector())) - deltaTime*glm::vec3(0.0f, this->getGravity(), 0.0f)); // This isn't how actually gravity works but it good enough for now
            
            if(this->getPosition().y < 0.0f) {
                this->damage(1, true);
            }
        }
};

#endif