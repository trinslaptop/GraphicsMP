#ifndef WORLD_HPP
#define WORLD_HPP

/*
 * World.hpp
 * Trin Wasinger - Fall 2025
 *
 * Stores and draws static object "blocks" in world
 */

#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "mcmodel.hpp"
#include "Block.hpp"

// Allow hashing glm::ivec3 
namespace std {
    namespace {
        template <class T> inline void hash_combine(std::size_t & s, const T & v) {
            std::hash<T> h;
            s^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
        }
    }

    template<> struct hash<glm::ivec3> {
        inline std::size_t operator()(const glm::ivec3& c) const {
            std::size_t result = 0;
            hash_combine(result, c.x);
            hash_combine(result, c.y);
            hash_combine(result, c.z);
            return result;
        }
    };
}

/// Represents the blocks in the world (and eventually other stuff?)
class World final : public mcmodel::Drawable {
    private:
        // This world is very sparse, so store as a map, also allows placing stuff outside a fixed region
        std::unordered_map<glm::ivec3, std::shared_ptr<Block>, std::hash<glm::ivec3>> _blocks;
    public:
        // World is 64x64
        static constexpr GLfloat WORLD_SIZE = 64.0f;

        inline World() : _blocks() {}
        inline virtual ~World() = default;

        // Non-Copyable
        World(const World&) = delete;
        World& operator=(const World&) = delete;

        /// Sets the block at a position or removes it if nullptr
        inline void setBlock(const glm::ivec3& pos, std::shared_ptr<Block> block = nullptr) {
            if(block) {
                this->_blocks[pos] = block;
            } else {
                this->_blocks.erase(pos);
            }
        }

        /// Gets the block at a position or nullptr if none
        inline std::shared_ptr<Block> getBlock(const glm::ivec3& pos) const {
            const auto iter = this->_blocks.find(pos);
            if(iter != this->_blocks.end()) {
                return iter->second;
            } else {
                return nullptr;
            }
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            for(const auto& entry : this->_blocks) {
                if(entry.second != nullptr) {
                    ctx.pushTransformation(glm::translate(glm::mat4(1.0), glm::vec3(entry.first)));
                    entry.second->draw(ctx);
                    ctx.popTransformation();
                }
            }
        }
};

#endif