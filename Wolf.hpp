#ifndef WOLF_HPP
#define WOLF_HPP

#include <memory>

#include <glm/glm.hpp>

#include "Entity.hpp"
#include "Particle.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "mcmodel.hpp"
#include "glutils.hpp"
#include "include/f8.hpp"

/// uwu
class Wolf final : public Entity {
    private:
        std::shared_ptr<mcmodel::Drawable> _head, _body, _tail, _right_front_leg, _left_front_leg, _right_back_leg, _left_back_leg, _root;
        const ShaderProgram& _shader;
        std::shared_ptr<Player> _target = nullptr;

    public:
        inline Wolf(World& world, const ShaderProgram& shader, const std::array<GLuint, 2> textures) : Entity(world), _shader(shader) {
            this->_root = mcmodel::group({
                this->_body = mcmodel::group({
                    mcmodel::group({
                        this->_head = mcmodel::group({
                            mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25f, 0.375f, 0.375f), glm::vec2(4.0f, 2.0f), glm::vec2(0.0f, 0.6875f)),
                            mcmodel::group({
                                mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25f, 0.1875f, 0.1875f), glm::vec2(4.0f, 2.0f), glm::vec2(0.0f, 0.46875f))
                            }, glm::vec3(0.1875f, -0.09375f, 0.0f)),
                            mcmodel::group({
                                mcmodel::group({
                                    mcmodel::wrapped_cube(shader, textures, glm::vec3(0.0625f, 0.125f, 0.125f), glm::vec2(4.0f, 2.0f), glm::vec2(0.25f, 0.46875f))
                                }, glm::vec3(0.0f, 0.0f, 0.125f)),
                                mcmodel::group({
                                    mcmodel::wrapped_cube(shader, textures, glm::vec3(0.0625f, 0.125f, 0.125f), glm::vec2(4.0f, 2.0f), glm::vec2(0.25f, 0.46875f))
                                }, glm::vec3(0.0f, 0.0f, -0.125f))
                            }, glm::vec3(-0.09375f, 0.25f, 0.0f))
                        }, glm::vec3(0.0f, -0.5f, 0.0f))
                    }, glm::vec3(0.78125f, 0.53125f, 0.0f)),
                    mcmodel::group({
                        mcmodel::group({
                            mcmodel::wrapped_cube(shader, textures, glm::vec3(0.4375f, 0.375f, 0.5f), glm::vec2(4.0f, 2.0f), glm::vec2(0.328125f, 0.59375f)),
                        }, glm::vec3(-0.03125f, 0.46875f, 0.0f)),
                        mcmodel::wrapped_cube(shader, textures, glm::vec3(0.375f, 0.5625f, 0.375f), glm::vec2(4.0f, 2.0f), glm::vec2(0.28125f, 0.09375f)),
                        this->_tail =  mcmodel::group({
                            mcmodel::wrapped_cube(shader, textures, glm::vec3(0.125f, 0.5f, 0.125f), glm::vec2(4.0f, 2.0f), glm::vec2(0.140625f, 0.125f))
                        }, glm::vec3(0.0f, -0.5f, 0.0f))
                    }, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -glutils::PI/2.0f))
                }, glm::vec3(-0.15625f, 0.1875f, 0.0f)),
                mcmodel::group({
                    this->_right_front_leg = mcmodel::group({
                        mcmodel::wrapped_cube(shader, textures, glm::vec3(0.125f, 0.5f, 0.125f), glm::vec2(4.0f, 2.0f), glm::vec2(0.0f, 0.125f))
                    }, glm::vec3(0.0f, -0.25f, 0.0f))
                }, glm::vec3(0.375f, 0.0f, 0.09375f)),
                mcmodel::group({
                    this->_left_front_leg = mcmodel::group({
                        mcmodel::wrapped_cube(shader, textures, glm::vec3(0.125f, 0.5f, 0.125f), glm::vec2(4.0f, 2.0f), glm::vec2(0.0f, 0.125f))
                    }, glm::vec3(0.0f, -0.25f, 0.0f))
                }, glm::vec3(0.375f, 0.0f, -0.09375f)),
                mcmodel::group({
                    this->_right_back_leg = mcmodel::group({
                        mcmodel::wrapped_cube(shader, textures, glm::vec3(0.125f, 0.5f, 0.125f), glm::vec2(4.0f, 2.0f), glm::vec2(0.0f, 0.125f))
                    }, glm::vec3(0.0f, -0.25f, 0.0f))
                }, glm::vec3(-0.3125f, 0.0f, 0.09375f)),
                mcmodel::group({
                    this->_left_back_leg = mcmodel::group({
                        mcmodel::wrapped_cube(shader, textures, glm::vec3(0.125f, 0.5f, 0.125f), glm::vec2(4.0f, 2.0f), glm::vec2(0.0f, 0.125f))
                    }, glm::vec3(0.0f, -0.25f, 0.0f))
                }, glm::vec3(-0.3125f, 0.0f, -0.09375f))
            }, glm::vec3(0.0f, 0.5f, 0.0f));
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            Entity::draw(ctx);
            if(!this->isHidden()) {
                this->_shader.setProgramUniform("tint", glm::vec4(glm::mix(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.33f, 0.33f), glm::cos(glutils::PI*(this->getHurtTime() - Entity::HURT_DURATION/2.0f)/Entity::HURT_DURATION)), 1.0f));

                ctx.pushTransformation(glm::translate(glm::mat4(1.0f), this->getPosition())*glm::yawPitchRoll(this->getRotation().x, this->getRotation().y, this->getRotation().z));
                    this->_root->draw(ctx);
                ctx.popTransformation();

                this->_shader.setProgramUniform("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        }

        inline virtual void update(const float deltaTime) override {
            Entity::update(deltaTime);

            // Track target
            if(this->getTarget()) {
                const glm::vec3 v = this->getTarget()->getPosition() - this->getPosition();
                const float dyaw = glm::mod(-glm::atan2(v.z, v.x) - this->getRotation().x + glutils::PI, 2.0f*glutils::PI) - glutils::PI;

                // Swap sign based on direction and clamp to prevent jitter
                this->setRotation(this->getRotation() + glm::vec3(glm::clamp(glm::sign(dyaw)*deltaTime, -glm::abs(dyaw), glm::abs(dyaw)), 0.0f, 0.0f));
            }

            // Animate
            std::dynamic_pointer_cast<mcmodel::Group>(this->_head)->rotation.x = 0.125f*std::cos(this->getLifetime() + 0.25f);

            std::dynamic_pointer_cast<mcmodel::Group>(this->_tail)->rotation.z = 0.1f*std::cos(2.0f*this->getLifetime() + 0.50f);
            std::dynamic_pointer_cast<mcmodel::Group>(this->_tail)->rotation.y = 0.1f*std::cos(this->getLifetime());
            
            std::dynamic_pointer_cast<mcmodel::Group>(this->_left_back_leg)->rotation.z = std::dynamic_pointer_cast<mcmodel::Group>(this->_right_front_leg)->rotation.z = -(std::dynamic_pointer_cast<mcmodel::Group>(this->_left_front_leg)->rotation.z = std::dynamic_pointer_cast<mcmodel::Group>(this->_right_back_leg)->rotation.z = 2.0f*glm::cos(this->getLimbSwing(deltaTime))*this->getLimbSwingAmount(deltaTime));
            
            std::dynamic_pointer_cast<mcmodel::Group>(this->_root)->rotation.y = 0.05f*glm::cos(glutils::PI*(this->getHurtTime() - Entity::HURT_DURATION/2.0f)/Entity::HURT_DURATION);

            if(glm::distance(glm::vec2(this->getPosition().x, this->getPosition().z), glm::vec2(this->getLastPosition().x, this->getLastPosition().z)) > 0.01f && f8::randb(0.2f) && !this->isInAir()) {
                this->getWorld().add(std::make_shared<DustParticle>(this->getPosition(), this->getRadius()));
            }

            if(this->getHealth() <= 0) {
                this->setPosition(this->getTarget() ? this->getTarget()->getPosition() : glm::vec3(1.0f, 0.0f, 1.0f));
            }
        }

        inline virtual float getHeight() const override {
            return 1.0f;
        }

        inline virtual float getEyeHeight() const override {
            return 0.75f;
        }

        inline virtual float getRadius() const override {
            return 0.5f;
        }

        inline virtual int getMaxHealth() const override {
            return 7;
        }

        inline void setTarget(std::shared_ptr<Player> target = nullptr) {
            this->_target = target;
        }

        /// Gets targeted player, could be nullptr!
        inline std::shared_ptr<Player> getTarget() const {
            return this->_target;
        }

        inline virtual const glm::vec3 getVelocity() const override {
            // Slow down when near to not overshoot
            return this->getTarget() ? glm::vec3(glm::min(2.0f*glm::distance(this->getTarget()->getPosition(), this->getPosition()), 2.0f), 0.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 0.0f);
        }
};

#endif