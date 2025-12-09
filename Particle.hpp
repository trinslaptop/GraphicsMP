#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <string>

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

#endif