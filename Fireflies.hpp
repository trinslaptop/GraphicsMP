#ifndef FIREFLIES_HPP
#define FIREFLIES_HPP

#include <vector>
#include <map>
#include <iterator>
#include <cstddef>

#include <glm/glm.hpp>

#include "include/f8.hpp"
#include "Particle.hpp"
#include "Particle.hpp"
#include "World.hpp"
#include "glutils.hpp"

/// A single firefly
class Firefly final : public SpriteParticle {
    private:
        const char _variant;
        const float _phase, _bonus;
        const glm::vec3 _velocity;
    protected:
        inline virtual float getMaxLifetime() const override final {
            return 3.5f + this->_bonus;
        }
        inline virtual const char getSprite() const override final {
            return '\xc1' + this->_variant;
        }
        inline virtual glm::vec3 getVelocity() const override final {
            return this->_velocity;
        }
        inline virtual float getSize() const override final {
            return 0.35f;
        }
        inline virtual bool isLit() const override final {
            return false;
        }
        inline virtual glm::vec3 getTint() const override final {
            return glm::mix(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::pow(glm::cos(this->getLifetime() + this->_phase), 2));
        }
    public:
        inline Firefly(const glm::vec3 position) :
            SpriteParticle(position + glm::vec3(2.0f*f8::randf() - 1.0f, 2.0f*f8::randf() - 1.0f, 2.0f*f8::randf() - 1.0f)),
            _velocity(0.75f*glm::vec3(f8::randf() - 0.5f, f8::randf() - 0.5f, f8::randf() - 0.5f)),
            _variant(f8::randi(0,2)),
            _phase(f8::randf()),
            _bonus(0.5f*f8::randf())
        {}
};

/// A swarm of fireflies that move along a bezier curve, spawns individual fireflies along it's path
class Fireflies final : public Particle {
    private:
        const std::vector<glm::vec3> _points;
        World& _world;
        double _lifetime = 0.0f;
        const float _speed;

        std::map<float, float> _arclut;
        float _arclength = 0.0f;
    public:
        /// Gets the number of complete curves
        inline size_t getCurveCount() const {
            return (this->_points.size() - !!this->_points.size())/3;
        }
        
        /// Gets the number of meaningful points (not strays for an incomplete curve)
        inline size_t getPointCount() const {
            return this->getCurveCount()*3 + !!this->getCurveCount();
        }

        /// Gets the approximated arc length
        inline float getArcLength() const {
            return this->_arclength;
        }
    protected:
        /// Evaluates the stored bezier curve at a given value after wrapping to the number of curves
        inline glm::vec3 evaluate(const float t) const {
            float c, v = glm::modf(glm::mod(t, (float) this->getCurveCount()), c);
            return glutils::bc(this->_points[3*c], this->_points[3*c + 1], this->_points[3*c + 2], this->_points[3*c + 3], v);
        }

        /// Approximates t for a given arc length after wrapping to total length
        inline float s2t(const float s) const {
            const float v = glm::mod(s, this->_arclength);
            const auto highp = this->_arclut.lower_bound(v), lowp = std::prev(highp);
            const float lows = (highp == this->_arclut.end() || highp == this->_arclut.begin()) ? 0.0f : lowp->first,  highs = (highp == this->_arclut.end()) ? this->_arclength : highp->first;
            const float lowt = (highp == this->_arclut.end() || highp == this->_arclut.begin()) ? 0.0f : lowp->second, hight = (highp == this->_arclut.end()) ? this->getCurveCount() : highp->second;

            return glm::mix(lowt, hight, glutils::safediv(v - lows, highs - lows));
        }

    public:
        template<typename T> inline Fireflies(World& world, const T& points, const float speed = 1.0f) : Particle(), _world(world), _points(points), _speed(speed) {
            // Build arc length parameterization LUT
            glm::vec3 last = this->evaluate(0.0f);
            for(float t = 0; t < this->getCurveCount(); t += 0.01f) {
                const glm::vec3 pos = this->evaluate(t);
                this->_arclut[this->_arclength += glm::distance(pos, last)] = t;
                last = pos;
            }
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override final {
            if(ctx.debug()) {
                for(size_t i = 0; i < this->getPointCount(); i++) {
                    if(i%3) {
                        // Show control points
                        ctx.getPrimitiveRenderer().point(this->_points[i], glm::vec3(0.0f, 1.0f, 0.0f));
                    } else {
                        // Render debug curve markers green if colinear with neighbors (aka C(1) continuous)
                        const glm::vec3 color = i > 0 && i + 1 < this->getPointCount() && glm::length(glm::cross(this->_points[i] - this->_points[i - 1], this->_points[i + 1] - this->_points[i - 1])) < 0.01f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
                        ctx.getPrimitiveRenderer().sprite(glutils::format("%x", (i/3)%16), this->_points[i], color, 0.5f, glutils::PrimitiveRenderer::SpriteMode::PARTICLE);
                    }

                    if(i) {
                        // Draw lines between control points to form cage
                        ctx.getPrimitiveRenderer().line(this->_points[i - 1], this->_points[i], glm::vec3(0.0f, 0.0f, 1.0f));
                    }
                }

                /*
                // Show arc length parameterization
                // (Disabled for now since it's a lot of points!)
                for(float s = 0.0f; s < this->getArcLength(); s += 0.5f) {
                    ctx.getPrimitiveRenderer().point(this->evaluate(this->s2t(s)), glm::vec3(1.0f, 0.0f, 1.0f), 0.5f);
                }

                for(const auto& entry : this->_arclut) {
                    ctx.getPrimitiveRenderer().point(this->evaluate(entry.second), glm::vec3(0.0f, 1.0f, 1.0f), 0.5f);
                }
                */

                // Show current position
                ctx.getPrimitiveRenderer().point(this->evaluate(this->s2t(this->_speed*this->_lifetime)), glm::vec3(1.0f, 1.0f, 0.0f), 2.0f);
            }
        }

        inline virtual void update(const float deltaTime) override final {
            this->_lifetime += deltaTime;

            if(f8::randb(0.2f)) {
                this->_world.add(std::make_shared<Firefly>(this->evaluate(this->s2t(this->_speed*this->_lifetime))));
            }
        }

        /// Loads a curve from JSON, see README
        inline static std::shared_ptr<Fireflies> from_json(World& world, const std::any& data) {
            auto root = json::cast::object(data);
            std::vector<glm::vec3> points;

            for(const auto& entry : json::cast::list(root["points"])) {
                const auto& point = json::cast::list(entry);
                points.push_back(glm::vec3(json::cast::number(point[0]), json::cast::number(point[1]), json::cast::number(point[2])));
            }

            return std::make_shared<Fireflies>(world, points, json::is::empty(root["speed"]) ? 1.0f : json::cast::number(root["speed"]));
        }
};

#endif