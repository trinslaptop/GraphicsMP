#include "ArcballCamera.hpp"

/*
 * (c) Trin Wasinger 2025
 * See ArcballCamera.hpp
 */

#include <algorithm>
#include <cmath>

/// bounds is a [min, max] for the radius
ArcballCamera::ArcballCamera(const glm::vec2 bounds) : CSCI441::PerspectiveCamera(), _bounds(bounds) {
    this->mCameraRadius = (bounds.x + bounds.y)/2.0f;
    this->recomputeOrientation();
}

void ArcballCamera::recomputeOrientation() {
    this->mCameraDirection = glm::vec3(
        this->mCameraRadius*std::sin(this->mCameraTheta)*std::sin(this->mCameraPhi),
        -this->mCameraRadius*std::cos(this->mCameraPhi),
        -this->mCameraRadius*std::cos(this->mCameraTheta)*std::sin(this->mCameraPhi)
    );
    this->setPosition(this->mCameraLookAtPoint + this->mCameraDirection);
    this->computeViewMatrix();
}

/// aka decrease radius
void ArcballCamera::moveForward(GLfloat movementFactor) {
    this->mCameraRadius = std::clamp(this->mCameraRadius + movementFactor, this->_bounds.x, this->_bounds.y);
    this->recomputeOrientation();
}

//s/ aka increase radius
void ArcballCamera::moveBackward(GLfloat movementFactor) {
    this->mCameraRadius = std::clamp(this->mCameraRadius - movementFactor, this->_bounds.x, this->_bounds.y);
    this->recomputeOrientation();
}