#ifndef ARCBALLCAMERA_HPP
#define ARCBALLCAMERA_HPP

/*
 * ArcballCamera.hpp
 * Trin Wasinger - Fall 2025
 *
 * A camera that orbits a look-at point
 */

#include <CSCI441/PerspectiveCamera.hpp>

#include <glm/glm.hpp>

class ArcballCamera final : public CSCI441::PerspectiveCamera {
    private:
        const glm::vec2 _bounds;
    public:
        /// bounds is a [min, max] for the radius, inital radius is centered in bounds
        ArcballCamera(const glm::vec2 bounds);
        virtual ~ArcballCamera() = default;

        // Non-Copyable
        ArcballCamera(const ArcballCamera&) = delete;
        ArcballCamera& operator=(const ArcballCamera&) = delete;

        void recomputeOrientation() override;

        /// aka decrease radius
        void moveForward(GLfloat movementFactor) override;

        /// aka increase radius
        void moveBackward(GLfloat movementFactor) override;
};

#endif