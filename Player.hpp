#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <memory>
#include <array>
#include <optional>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include <CSCI441/FixedCam.hpp>

#include "ShaderProgram.hpp"
#include "World.hpp"
#include "mcmodel.hpp"
#include "glutils.hpp"

#include "ArcballCamera.hpp"

/*
 * Player.hpp
 * Trin Wasinger - Fall 2025
 *
 * The player character composed of position, rotation, model, and cameras. Handles
 * collision and animation
 * 
 * Texture should be a minecraft player skin, cape can optionally be a minecraft cape texture
 */

class Player final : public mcmodel::Drawable {
    private:
        std::shared_ptr<mcmodel::Drawable> _head, _body, _right_arm, _left_arm, _right_leg, _left_leg, _cape, _root;
        
        // Time accumulator
        GLfloat _acctime = 0.0f;

        glm::vec3 _position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 _rotation = glm::vec3(0.0f, 0.0f, 0.0f);

        ArcballCamera _arcballcamera;
        CSCI441::FixedCam _skycamera;
        CSCI441::FixedCam _fpcamera;

        std::shared_ptr<World> _world;

        bool _hidden = false;
        
    public:
        Player(std::shared_ptr<World> world, const ShaderProgram& shader, const std::array<GLuint, 2> textures, bool thinArms = true, const std::optional<const std::array<GLuint, 2>> cape = std::nullopt) : _world(world), _arcballcamera({1.0f, 6.0f}), _skycamera(), _fpcamera() {
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

        inline virtual ~Player() = default;

        // Non-Copyable
        Player(const Player&) = delete;
        Player& operator=(const Player&) = delete;
    
        inline void setPosition(const glm::vec3 position, bool ignoreCollision = false) {
            const glm::vec3 clamped = glm::clamp(position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(World::WORLD_SIZE, World::WORLD_SIZE, World::WORLD_SIZE));
        
            // This is a very cheep hack for trivial collision
            // Effectively uses the space from not moving as far as possible but only to a position if result is clear
            // as the collision radius/box for the player (only works well for low speeds!) 
            std::shared_ptr<Block> block;
            if(ignoreCollision || (((block = this->_world->getBlock(glm::ivec3(clamped))) == nullptr || !block->isSolid()) && ((block = this->_world->getBlock(glm::ivec3(clamped + glm::vec3(0.0f, 1.0f, 0.0f)))) == nullptr || !block->isSolid()))) {
                this->_position = clamped;
                
                // Update cameras
                this->_arcballcamera.setLookAtPoint(this->_position + glm::vec3(0.0f, 1.75f, 0.0f));
                this->_arcballcamera.recomputeOrientation();

                this->_skycamera.setPosition(this->_position + glm::vec3(0.0f, 12.0f, 0.0f));
                this->_skycamera.setLookAtPoint(this->_position);
                this->_skycamera.setUpVector(this->getHorizontalForwardVector());
                this->_skycamera.computeViewMatrix();

                this->_fpcamera.setPosition(this->_position + 0.5f*this->getForwardVector() + glm::vec3(0.0f, 1.75f, 0.0f));
                this->_fpcamera.setLookAtPoint(this->_position + 2.0f*this->getForwardVector() + glm::vec3(0.0f, 1.75f, 0.0f));
                this->_fpcamera.setUpVector(this->getUpVector());
                this->_fpcamera.computeViewMatrix();
            }
        }

        inline glm::vec3 getPosition() const {
            return this->_position;
        }

        inline void setRotation(const glm::vec3 rotation) {
            this->_rotation = glm::mod(rotation, 2*glutils::PI);

            // Update cameras
            this->_skycamera.setPosition(this->_position + glm::vec3(0.0f, 12.0f, 0.0f));
            this->_skycamera.setLookAtPoint(this->_position);
            this->_skycamera.setUpVector(this->getHorizontalForwardVector());
            this->_skycamera.computeViewMatrix();

            this->_fpcamera.setPosition(this->_position + glm::vec3(0.0f, 1.75f, 0.0f));
            this->_fpcamera.setLookAtPoint(this->_position + 2.0f*this->getForwardVector() + glm::vec3(0.0f, 1.75f, 0.0f));
            this->_fpcamera.setUpVector(this->getUpVector());
            this->_fpcamera.computeViewMatrix();
        }

        inline glm::vec3 getRotation() const {
            return this->_rotation;
        }

        inline ArcballCamera& getArcballCamera() {
            return this->_arcballcamera;
        }

        inline CSCI441::FixedCam& getFirstPersonCamera() {
            return this->_fpcamera;
        }

        inline CSCI441::FixedCam& getSkyCamera() {
            return this->_skycamera;
        }

        // An xz aligned vector representing direction of player
        inline glm::vec3 getHorizontalForwardVector() const {
            return glm::vec3(glm::cos(this->_rotation.x), 0.0f, -glm::sin(this->_rotation.x));
        }

        inline glm::vec3 getForwardVector() const {
            return glm::vec3(glm::yawPitchRoll(this->_rotation.x, this->_rotation.y, this->_rotation.z)*glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
        }

        inline glm::vec3 getUpVector() const {
            return glm::vec3(glm::yawPitchRoll(this->_rotation.x, this->_rotation.y, this->_rotation.z)*glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
        }

        inline void update(GLfloat deltaTime) {
            // Update and wrap time accumulator
            this->_acctime += deltaTime;
            if(this->_acctime > 2.0f*glutils::PI) {
                this->_acctime -= 2.0f*glutils::PI;
            }

            // Animate cape
            std::dynamic_pointer_cast<mcmodel::Group>(this->_cape)->rotation.z = glutils::PI/8.0f + 0.25f*std::cos(this->_acctime + 0.25f);

            // Animate head
            std::dynamic_pointer_cast<mcmodel::Group>(this->_head)->rotation.x = 0.125f*std::cos(this->_acctime + 0.50f);

            // Animate arms
            std::dynamic_pointer_cast<mcmodel::Group>(this->_right_arm)->rotation.z = -(std::dynamic_pointer_cast<mcmodel::Group>(this->_left_arm)->rotation.z = 0.25f*std::cos(this->_acctime));
            
            // Animate legs
            std::dynamic_pointer_cast<mcmodel::Group>(this->_right_leg)->rotation.z = -(std::dynamic_pointer_cast<mcmodel::Group>(this->_left_leg)->rotation.z = glutils::PI/8.0f * glm::sin(3.0f*glm::length(this->_position)));
        }

        inline void setHidden(const bool hidden) {
            this->_hidden = hidden;
        }

        inline bool isHidden() const {
            return this->_hidden;
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            if(!this->_hidden) {
                ctx.pushTransformation(glm::translate(glm::mat4(1.0f), this->_position)*glm::yawPitchRoll(this->_rotation.x, this->_rotation.y, this->_rotation.z));
                        this->_root->draw(ctx);
                ctx.popTransformation();
            }
        }
};
#endif