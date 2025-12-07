#ifndef ZOMBIE_HPP
#define ZOMBIE_HPP

#include <memory>

#include "Entity.hpp"
#include "Player.hpp"

class Zombie final : public Entity {
    private:
        std::shared_ptr<mcmodel::Drawable> _head, _body, _right_arm, _left_arm, _right_leg, _left_leg, _root;
        GLfloat _acctime = 0.0f;

        std::shared_ptr<Player> _target = nullptr;

    public:
        inline Zombie(const std::shared_ptr<World> world, const ShaderProgram& shader, const std::array<GLuint, 2> textures) : Entity(world) {
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
            ctx.getPrimitiveRenderer().point(this->getPosition(), {1,0,0});
            if(!this->isHidden()) {
                ctx.pushTransformation(glm::translate(glm::mat4(1.0f), this->getPosition())*glm::yawPitchRoll(this->getRotation().x, this->getRotation().y, this->getRotation().z));
                    this->_root->draw(ctx);
                ctx.popTransformation();
            }
        };

        inline virtual void update(const float deltaTime) {
            // fprintf(stderr, "Touching: %d", this->isTouching(this->_target));
        };

        inline virtual float getHeight() const override {
            return 2.0f;
        }

        inline virtual float getEyeHeight() const override {
            return 1.5f;
        }

        inline virtual float getRadius() const override {
            return 0.25f;
        }

        inline void setTarget(std::shared_ptr<Player> target = nullptr) {
            this->_target = target;
        }

        inline std::shared_ptr<Player> getTarget() const {
            return this->_target;
        }
};

#endif