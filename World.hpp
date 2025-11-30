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

#include "NonCopyable.hpp"

#include "f8.hpp"

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

/// Cubic Bezier Curve formula
template<typename T> inline T bc(const T p0, const T p1, const T p2, const T p3, const float t) {
    return glm::pow(1 - t, 3)*p0 + 3*glm::pow(1 - t, 2)*t*p1 + 3*(1-t)*glm::pow(t, 2)*p2 + glm::pow(t, 3)*p3;
} 

class Chunk final : public mcmodel::Drawable, NonCopyable  {
    public:
        static constexpr float CHUNK_SIZE = 16;
    public:
        const glm::ivec3 _chunk_pos;

        const std::array<glm::vec3, 16> _terrain;

        // The world is very sparse, so store as a map
        std::unordered_map<glm::ivec3, std::shared_ptr<Block>, std::hash<glm::ivec3>> _blocks;

        const ShaderProgram& _tex_shader;
        const ShaderProgram& _terrain_shader;

        const GLuint _vao;
        const GLuint _vbo;
        const std::array<GLuint, 2> _textures;
    public:
        inline Chunk(const ShaderProgram& tex_shader, const ShaderProgram& terrain_shader, const glm::ivec3 chunk_pos, const std::array<glm::vec3, 16> terrain, const std::array<GLuint, 2> textures, GLuint vao, GLuint vbo) :
            _tex_shader(tex_shader),
            _terrain_shader(terrain_shader),
            _chunk_pos(chunk_pos),
            _blocks(),
            _terrain(terrain),
            _textures(textures),
            _vao(vao),
            _vbo(vbo)
        {}

        inline virtual ~Chunk() = default;

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            // Draw terrain
            ctx.bind(this->_terrain_shader);
            glPatchParameteri(GL_PATCH_VERTICES, 16);

            this->_terrain_shader.setProgramUniform("size", Chunk::CHUNK_SIZE);
            this->_terrain_shader.setProgramUniform("tint", glm::vec4(0.19f, 0.5f, 0.0f, 1.0f));

            for(size_t i = 0; i < sizeof(this->_textures)/sizeof(this->_textures[0]); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, this->_textures[i]);
            }

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_PATCHES, 0, 16);

            // Reset
            this->_terrain_shader.setProgramUniform("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            
            // Draw blocks
            ctx.bind(this->_tex_shader);
            for(const auto& entry : this->_blocks) {
                if(entry.second != nullptr) {
                    ctx.pushTransformation(glm::translate(glm::mat4(1.0), glm::vec3(this->_chunk_pos)*Chunk::CHUNK_SIZE + glm::vec3(entry.first)));
                    entry.second->draw(ctx);
                    ctx.popTransformation();
                }
            }
        }

        inline float getTerrainHeight(const float x, const float z) const {
            return bc(
                bc(this->_terrain[0x0], this->_terrain[0x1], this->_terrain[0x2], this->_terrain[0x3], glm::mod(x, Chunk::CHUNK_SIZE)/CHUNK_SIZE),
                bc(this->_terrain[0x4], this->_terrain[0x5], this->_terrain[0x6], this->_terrain[0x7], glm::mod(x, Chunk::CHUNK_SIZE)/CHUNK_SIZE),
                bc(this->_terrain[0x8], this->_terrain[0x9], this->_terrain[0xa], this->_terrain[0xb], glm::mod(x, Chunk::CHUNK_SIZE)/CHUNK_SIZE),
                bc(this->_terrain[0xc], this->_terrain[0xd], this->_terrain[0xe], this->_terrain[0xf], glm::mod(x, Chunk::CHUNK_SIZE)/CHUNK_SIZE),
                glm::mod(z, Chunk::CHUNK_SIZE)/CHUNK_SIZE
            ).y;
        }

        inline glm::vec3 getTerrainNormal(const float x, const float z) const {
            return glm::vec3(0.0f);
        }

        inline static std::shared_ptr<Chunk> from(
            const ShaderProgram& tex_shader,
            const ShaderProgram& terrain_shader,
            const glm::ivec3 chunk_pos,
            const unsigned int seed,
            const std::array<GLuint, 2> textures
        ) {
            glm::vec2 state;
            f8::vsrand(seed, state);

            // For simplicity, we'll fix xz
            std::array<glm::vec3, 16> terrain {
                glm::vec3(0.0f, 0.0f,              0.0f), glm::vec3(CHUNK_SIZE/3.0f, 0.0f,              0.0f), glm::vec3(2*CHUNK_SIZE/3.0f, 0.0f,              0.0f), glm::vec3(CHUNK_SIZE, 0.0f,              0.0f),
                glm::vec3(0.0f, 0.0f,   CHUNK_SIZE/3.0f), glm::vec3(CHUNK_SIZE/3.0f, 0.0f,   CHUNK_SIZE/3.0f), glm::vec3(2*CHUNK_SIZE/3.0f, 0.0f,   CHUNK_SIZE/3.0f), glm::vec3(CHUNK_SIZE, 0.0f,   CHUNK_SIZE/3.0f),
                glm::vec3(0.0f, 0.0f, 2*CHUNK_SIZE/3.0f), glm::vec3(CHUNK_SIZE/3.0f, 0.0f, 2*CHUNK_SIZE/3.0f), glm::vec3(2*CHUNK_SIZE/3.0f, 0.0f, 2*CHUNK_SIZE/3.0f), glm::vec3(CHUNK_SIZE, 0.0f, 2*CHUNK_SIZE/3.0f),
                glm::vec3(0.0f, 0.0f,        CHUNK_SIZE), glm::vec3(CHUNK_SIZE/3.0f, 0.0f,        CHUNK_SIZE), glm::vec3(2*CHUNK_SIZE/3.0f, 0.0f,        CHUNK_SIZE), glm::vec3(CHUNK_SIZE, 0.0f,        CHUNK_SIZE),
            };

            for(glm::vec3& pos : terrain) {
                // Convert from inner chunk coordinates to world coordinates
                pos += glm::vec3(chunk_pos)*Chunk::CHUNK_SIZE;

                // Randomize y-value
                pos.y += 5*f8::fbm(glm::vec2(pos.x, pos.z), 6, state);
            }

            GLuint vao, vbo;

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(terrain), terrain.data(), GL_STATIC_DRAW);

            GLint attrloc;
            if((attrloc = terrain_shader.getAttributeLocation(STRC(vPos))) != -1) {
                glEnableVertexAttribArray(attrloc);
                glVertexAttribPointer(attrloc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            }

            return std::make_shared<Chunk>(tex_shader, terrain_shader, chunk_pos, terrain, textures, vao, vbo);
        }
};

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