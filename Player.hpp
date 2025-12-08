#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <memory>
#include <array>
#include <optional>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include <CSCI441/FixedCam.hpp>
#include <CSCI441/ArcballCam.hpp>

#include "ShaderProgram.hpp"
#include "World.hpp"
#include "mcmodel.hpp"
#include "glutils.hpp"
#include "Entity.hpp"

/*
 * Player.hpp
 * Trin Wasinger - Fall 2025
 *
 * The player character composed of position, rotation, model, and cameras. Handles
 * collision and animation
 * 
 * Texture should be a minecraft player skin, cape can optionally be a minecraft cape texture
 */

class Player final : public Entity {
    private:
        std::shared_ptr<mcmodel::Drawable> _head, _body, _right_arm, _left_arm, _right_leg, _left_leg, _cape, _root;
        
        const ShaderProgram& _shader;

        CSCI441::ArcballCam _arcballcamera;
        CSCI441::FixedCam _skycamera;
        CSCI441::FixedCam _fpcamera;

        inline void _updateCameras() {
            this->_arcballcamera.setLookAtPoint(this->getEyePosition());
            this->_arcballcamera.recomputeOrientation();

            this->_skycamera.setPosition(this->getPosition() + glm::vec3(0.0f, 12.0f, 0.0f));
            this->_skycamera.setLookAtPoint(this->getPosition());
            this->_skycamera.setUpVector(this->getForwardVector());
            this->_skycamera.computeViewMatrix();

            this->_fpcamera.setPosition(this->getEyePosition());
            this->_fpcamera.setLookAtPoint(this->getEyePosition() + 2.0f*this->getForwardVector());
            this->_fpcamera.setUpVector(this->getUpVector());
            this->_fpcamera.computeViewMatrix();
        }

    public:
        Player(std::shared_ptr<World> world, const ShaderProgram& shader, const std::array<GLuint, 2> textures, bool thinArms = true, const std::optional<const std::array<GLuint, 2>> cape = std::nullopt) : Entity(world), _arcballcamera(1.0f, 6.0f), _skycamera(), _fpcamera(), _shader(shader) {
            this->_arcballcamera.moveBackward(2.5f);
            
            // Create player model
            this->_root = mcmodel::group({
                this->_body = mcmodel::group({
                    mcmodel::group({
                        this->_head = mcmodel::group({
                            mcmodel::wrapped_cube(shader, textures, glm::vec3(0.5, 0.5, 0.5f), glm::vec2(4.0f, 4.0f), glm::vec2(0.0f, 0.75f)),
                            mcmodel::group({mcmodel::wrapped_cube(shader, textures, glm::vec3(0.5, 0.5, 0.5f), glm::vec2(4.0f, 4.0f), glm::vec2(0.5f, 0.75f))}, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.1f, 1.1f, 1.1f))
                        }, glm::vec3(0.0f, 0.25f, 0.0f))
                    }, glm::vec3(0.0f, 0.375f, 0.0f)),
                    mcmodel::group({
                        this->_cape = cape ? mcmodel::group({
                            mcmodel::wrapped_cube(shader, cape.value(), glm::vec3(0.0625f, 1.0f, 0.625), glm::vec2(4.0f, 2.0f), glm::vec2(0.0f, 0.46875f))
                        }, glm::vec3(-0.03125f, -0.5f, 0.0f), glm::vec3(glutils::PI, 0.0f, 0.0f)) : mcmodel::group({})
                    }, glm::vec3(-0.1875f, 0.40625f, 0.0f)),
                    mcmodel::group({
                        this->_right_arm = mcmodel::group({
                            mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, thinArms ? 0.1875f : 0.25f), glm::vec2(4.0f, 4.0f), glm::vec2(0.625f, 0.5f)),
                            mcmodel::group({mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, thinArms ? 0.1875f : 0.25f), glm::vec2(4.0f, 4.0f), glm::vec2(0.625f, 0.25f))}, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.1f, 1.1f, 1.1f))
                        }, glm::vec3(0.0f, -0.375f, thinArms ? 0.34375 : 0.375f)),
                        this->_left_arm = mcmodel::group({
                            mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, thinArms ? 0.1875f : 0.25f), glm::vec2(4.0f, 4.0f), glm::vec2(0.5f, 0.0f)),
                            mcmodel::group({mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, thinArms ? 0.1875f : 0.25f), glm::vec2(4.0f, 4.0f), glm::vec2(0.75, 0.0f))}, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.1f, 1.1f, 1.1f))
                        }, glm::vec3(0.0f, -0.375f, thinArms ? -0.34375 : -0.375f))
                    }, glm::vec3(0.0f, 0.375f, 0.0f)),
                    mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.5f), glm::vec2(4.0f, 4.0f), glm::vec2(0.25f, 0.5f)),
                    mcmodel::group({mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.5f), glm::vec2(4.0, 4.0), glm::vec2(0.25f, 0.25f))}, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.1f, 1.1f, 1.1f))
                }, glm::vec3(0.0f, 0.375, 0.0f)),
                this->_right_leg = mcmodel::group({
                    mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.25), glm::vec2(4.0f, 4.0f), glm::vec2(0.0f, 0.5f)),
                    mcmodel::group({mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.25), glm::vec2(4.0f, 4.0f), glm::vec2(0.0f, 0.25f))}, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.1f, 1.1f, 1.1f))
                }, glm::vec3(0.0f, -0.375f, 0.125f)),
                this->_left_leg = mcmodel::group({
                    mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.25), glm::vec2(4.0f, 4.0f), glm::vec2(0.25f, 0.0f)),
                    mcmodel::group({mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.25), glm::vec2(4.0f, 4.0f), glm::vec2(0.0f, 0.0f))}, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.1f, 1.1f, 1.1f))
                }, glm::vec3(0.0f, -0.375f, -0.125f))
            }, glm::vec3(0.0f, 0.75f, 0.0f));
        }

        inline CSCI441::ArcballCam& getArcballCamera() {
            return this->_arcballcamera;
        }

        inline CSCI441::FixedCam& getFirstPersonCamera() {
            return this->_fpcamera;
        }

        inline CSCI441::FixedCam& getSkyCamera() {
            return this->_skycamera;
        }

        inline virtual void update(const GLfloat deltaTime) override {
            Entity::update(deltaTime);

            // Animate cape
            std::dynamic_pointer_cast<mcmodel::Group>(this->_cape)->rotation.z = glutils::PI/8.0f + 0.25f*std::cos(this->getLifetime() + 0.25f);

            // Animate head
            std::dynamic_pointer_cast<mcmodel::Group>(this->_head)->rotation.x = 0.125f*std::cos(this->getLifetime() + 0.50f);

            // Animate arms
            std::dynamic_pointer_cast<mcmodel::Group>(this->_right_arm)->rotation.z = -(std::dynamic_pointer_cast<mcmodel::Group>(this->_left_arm)->rotation.z = 0.25f*std::cos(this->getLifetime()));
            
            // Animate legs
            std::dynamic_pointer_cast<mcmodel::Group>(this->_right_leg)->rotation.z = -(std::dynamic_pointer_cast<mcmodel::Group>(this->_left_leg)->rotation.z = glutils::PI/8.0f * glm::sin(3.0f*glm::length(this->getPosition())));
        
            this->_updateCameras();
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            Entity::draw(ctx);
            if(!this->isHidden()) {
                this->_shader.setProgramUniform("tint", glm::vec4(glm::mix(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.33f, 0.33f), glm::cos(glutils::PI*(this->getHurtTime() - Entity::HURT_DURATION/2.0f)/Entity::HURT_DURATION)), 1.0f));

                ctx.pushTransformation(glm::translate(glm::mat4(1.0f), this->getPosition())*glm::yawPitchRoll(this->getRotation().x, this->getRotation().y, this->getRotation().z));
                        this->_root->draw(ctx);
                ctx.popTransformation();
            }
        }

        inline virtual float getHeight() const override {
            return 2.0f;
        }

        inline virtual float getEyeHeight() const override {
            return 1.75f;
        }

        inline virtual float getRadius() const override {
            return 0.25f;
        }

        inline virtual int getMaxHealth() const override {
            return 5;
        }

        inline virtual const glm::vec3 getVelocity() const override {
            return glm::vec3(0.0f, 0.0f, 0.0f);//TODO:
        }
};
#endif