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

    public:
        inline Particle() : _uuid(f8::uuid4()) {};
        inline virtual ~Particle() = default;
        inline virtual void update(const float deltaTime) = 0;

        inline virtual const std::string& getUUID() const noexcept final {
            return this->_uuid;
        }
};

#endif