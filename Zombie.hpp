#ifndef ZOMBIE_HPP
#define ZOMBIE_HPP

#include <memory>

#include <glm/glm.hpp>

#include "Entity.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "mcmodel.hpp"
#include "glutils.hpp"
#include "include/f8.hpp"

class Zombie final : public Entity {
    private:
        std::shared_ptr<mcmodel::Drawable> _head, _body, _right_arm, _left_arm, _right_leg, _left_leg, _root;
        const ShaderProgram& _shader;
        std::shared_ptr<Player> _target = nullptr;

    public:
        inline Zombie(World& world, const ShaderProgram& shader, const std::array<GLuint, 2> textures) : Entity(world), _shader(shader) {
            // Slightly modified from player model
            this->_root = mcmodel::group({
                this->_body = mcmodel::group({
                    mcmodel::group({
                        this->_head = mcmodel::group({
                            mcmodel::wrapped_cube(shader, textures, glm::vec3(0.5, 0.5, 0.5f), glm::vec2(4.0f, 4.0f), glm::vec2(0.0f, 0.75f))
                        }, glm::vec3(0.0f, 0.25f, 0.0f))
                    }, glm::vec3(0.0f, 0.375f, 0.0f)),
                    mcmodel::group({
                        this->_right_arm = mcmodel::group({
                            mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.25f), glm::vec2(4.0f, 4.0f), glm::vec2(0.625f, 0.5f))
                        }, glm::vec3(0.0f, -0.375f, 0.375f)),
                        this->_left_arm = mcmodel::group({
                            mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.25f), glm::vec2(4.0f, 4.0f), glm::vec2(0.5f, 0.0f))
                        }, glm::vec3(0.0f, -0.375f, -0.375f))
                    }, glm::vec3(0.0f, 0.375f, 0.0f)),
                    mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.5f), glm::vec2(4.0f, 4.0f), glm::vec2(0.25f, 0.5f))
                }, glm::vec3(0.0f, 0.375, 0.0f)),
                this->_right_leg = mcmodel::group({
                    mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.25), glm::vec2(4.0f, 4.0f), glm::vec2(0.0f, 0.5f))
                }, glm::vec3(0.0f, -0.375f, 0.125f)),
                this->_left_leg = mcmodel::group({
                    mcmodel::wrapped_cube(shader, textures, glm::vec3(0.25, 0.75, 0.25), glm::vec2(4.0f, 4.0f), glm::vec2(0.25f, 0.0f))
                }, glm::vec3(0.0f, -0.375f, -0.125f))
            }, glm::vec3(0.0f, 0.75f, 0.0f));
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

        inline virtual void update(const float deltaTime) override {
            Entity::update(deltaTime);

            if(this->getTarget()) {
                // Track target
                const glm::vec3 v = this->getTarget()->getPosition() - this->getPosition();
                this->setRotation({glm::mix(this->getRotation().x, -glm::atan2(v.z, v.x), 0.9f), this->getRotation().y, this->getRotation().z});
                
                if(this->isTouching(*this->getTarget()) && f8::randb(0.2f)) {
                    this->getTarget()->damage();
                }
            }

            // Animate
            std::dynamic_pointer_cast<mcmodel::Group>(this->_head)->rotation.x = 0.125f*std::cos(this->getLifetime() + 0.50f);
            std::dynamic_pointer_cast<mcmodel::Group>(this->_right_arm)->rotation.z = -(std::dynamic_pointer_cast<mcmodel::Group>(this->_left_arm)->rotation.z = 0.25f*std::cos(this->getLifetime()) + glutils::PI/2) + glutils::PI;
            std::dynamic_pointer_cast<mcmodel::Group>(this->_right_leg)->rotation.z = -(std::dynamic_pointer_cast<mcmodel::Group>(this->_left_leg)->rotation.z = glutils::PI/8.0f * glm::sin(3.0f*glm::length(this->getPosition())));

            if(this->getHealth() <= 0) {
                this->setDead();
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

        inline void setTarget(std::shared_ptr<Player> target = nullptr) {
            this->_target = target;
        }

        /// Gets targeted player, could be nullptr!
        inline std::shared_ptr<Player> getTarget() const {
            return this->_target;
        }

        inline virtual const glm::vec3 getVelocity() const override {
            return this->getTarget() ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 0.0f);
        }

        inline virtual bool hasInteraction() const {
            return true;
        }

        inline virtual void interact(Particle& other) {
            if(Zombie* zombie = dynamic_cast<Zombie*>(&other)) {
                if(this->isTouching(*zombie)) {
                    zombie->damage(1);
                }
            }
        }


};

#endif