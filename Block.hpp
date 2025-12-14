#ifndef BLOCK_HPP
#define BLOCK_HPP

/*
 * Block.hpp
 * Trin Wasinger - Fall 2025
 *
 * A static object in the world, holds model and collision
 */

#include "mcmodel.hpp"
#include "glutils.hpp"
#include "include/json.hpp"

#include <memory>

/// A class to combine model data with some gameplay data (e.g. collision)
/// All blocks of the same type in the world are the same instance of this class
class Block final : public mcmodel::Drawable {
    private:
        std::shared_ptr<mcmodel::Drawable> _model;
        bool _solid;
    public:
        inline Block(std::shared_ptr<mcmodel::Drawable> model, const bool solid = true) : _model(model), _solid(solid) {}
        inline virtual ~Block() = default;

        // Non-Copyable
        Block(const Block&) = delete;
        Block& operator=(const Block&) = delete;

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            this->_model->draw(ctx);
        }

        /// Should this block have collision
        inline bool isSolid() const {
            return this->_solid;
        }

        inline static std::shared_ptr<Block> from(std::shared_ptr<mcmodel::Drawable> model, const bool solid = true) {
            return std::make_shared<Block>(model, solid);
        }

        /// Loads a block from JSON, see README
        inline static std::shared_ptr<Block> from_json(const ShaderProgram &shader, glutils::TextureManager& tm, const std::any& data) {
            auto root = json::cast::object(data);
            return from(mcmodel::from_json(shader, tm, root["model"]), json::is::empty(root["collision"]) ? true : json::cast::boolean(root["collision"]));
        }
};

#endif