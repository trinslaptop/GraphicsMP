#ifndef WORLD_HPP
#define WORLD_HPP

/*
 * World.hpp
 * Trin Wasinger - Fall 2025
 *
 * Stores and draws "block" objects in world
 */

#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "mcmodel.hpp"
#include "Block.hpp"
#include "NonCopyable.hpp"
#include "Particle.hpp"

#include "include/f8.hpp"

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

/// A subregion of the world, if at y=0, also has terrain
class Chunk final : public mcmodel::Drawable, NonCopyable  {
    public:
        static constexpr float CHUNK_SIZE = 16;
    public:
        const glm::ivec3 _chunk_pos;

        const std::array<glm::vec3, 16> _terrain;

        // The world is very sparse, so store as a map
        std::unordered_map<glm::ivec3, std::shared_ptr<Block>, std::hash<glm::ivec3>> _blocks;

        const struct {
            const ShaderProgram& block;
            const ShaderProgram& terrain;
        } _shaders;

        const GLuint _vao;
        const GLuint _vbo;
        const std::array<GLuint, 2> _textures;
    public:
        inline Chunk(const ShaderProgram& block_shader, const ShaderProgram& terrain_shader, const glm::ivec3 chunk_pos, const std::array<glm::vec3, 16> terrain, const std::array<GLuint, 2> textures, GLuint vao, GLuint vbo) :
            _shaders{block_shader, terrain_shader},
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
            if(!this->_chunk_pos.y) {
                ctx.bind(this->_shaders.terrain);
                glPatchParameteri(GL_PATCH_VERTICES, 16);
    
                this->_shaders.terrain.setProgramUniform("size", Chunk::CHUNK_SIZE);
                this->_shaders.terrain.setProgramUniform("tint", glm::vec4(0.19f, 0.5f, 0.0f, 1.0f));
    
                for(size_t i = 0; i < sizeof(this->_textures)/sizeof(this->_textures[0]); i++) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, this->_textures[i]);
                }
    
                glBindVertexArray(this->_vao);
                glDrawArrays(GL_PATCHES, 0, 16);
    
                // Reset
                this->_shaders.terrain.setProgramUniform("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
            
            // Draw blocks
            ctx.bind(this->_shaders.block);
            for(const auto& entry : this->_blocks) {
                if(entry.second != nullptr) {
                    ctx.pushTransformation(glm::translate(glm::mat4(1.0), glm::vec3(this->_chunk_pos)*Chunk::CHUNK_SIZE + glm::vec3(entry.first)));
                    entry.second->draw(ctx);
                    ctx.popTransformation();
                }
            }
        }

        /// Gets the minimum y value for this chunk, only meaningful for bottom chunk of world
        inline float getTerrainHeight(const float x, const float z) const {
            return this->_chunk_pos.y ? -INFINITY : bc(
                bc(this->_terrain[0x0], this->_terrain[0x1], this->_terrain[0x2], this->_terrain[0x3], glm::mod(x, Chunk::CHUNK_SIZE)/CHUNK_SIZE),
                bc(this->_terrain[0x4], this->_terrain[0x5], this->_terrain[0x6], this->_terrain[0x7], glm::mod(x, Chunk::CHUNK_SIZE)/CHUNK_SIZE),
                bc(this->_terrain[0x8], this->_terrain[0x9], this->_terrain[0xa], this->_terrain[0xb], glm::mod(x, Chunk::CHUNK_SIZE)/CHUNK_SIZE),
                bc(this->_terrain[0xc], this->_terrain[0xd], this->_terrain[0xe], this->_terrain[0xf], glm::mod(x, Chunk::CHUNK_SIZE)/CHUNK_SIZE),
                glm::mod(z, Chunk::CHUNK_SIZE)/CHUNK_SIZE
            ).y;
        }

        inline glm::vec3 getTerrainNormal(const float x, const float z) const {
            // TODO: NYI
            return glm::vec3(0.0f);
        }

        /// Sets the block at a position or removes it if nullptr
        inline void setBlock(const glm::ivec3& offset, std::shared_ptr<Block> block = nullptr) {
            if(block) {
                this->_blocks[offset] = block;
            } else {
                this->_blocks.erase(offset);
            }
        }

        /// Gets the block at a position or nullptr if none
        inline std::shared_ptr<Block> getBlock(const glm::ivec3& offset) const {
            const auto iter = this->_blocks.find(offset);
            if(iter != this->_blocks.end()) {
                return iter->second;
            } else {
                return nullptr;
            }
        }

        inline static std::shared_ptr<Chunk> from(
            const ShaderProgram& block_shader,
            const ShaderProgram& terrain_shader,
            const glm::ivec3 chunk_pos,
            const unsigned int seed,
            const std::array<GLuint, 2> textures
        ) {
            if(!chunk_pos.y) {
                glm::vec2 state;
                f8::srandv(seed, state);

                // For the sake of not mixing up different "y", the below code uses the xz of a vec3 with y=0 instead of a vec2 when needed
                const auto sample = [&](const glm::vec3 pos) {
                    return 5.0f*f8::fbm(glm::vec2(pos.x, pos.z), 6, state) + 1.0f/16.0f;
                };

                // Use a centered difference to approximate the gradient of the noise (MATH 307)
                const auto grad = [&](const glm::vec3 pos, const float h = 0.0001f) {
                    return glm::vec3(
                        sample(pos + glm::vec3(h, 0, 0)) - sample(pos - glm::vec3(h, 0, 0)),
                        0.0f,
                        sample(pos + glm::vec3(0, 0, h)) - sample(pos - glm::vec3(0, 0, h))
                    )/(2.0f*h);
                };

                // For simplicity, we'll fix xz
                // We can also freely pick the y in the corners
                // All other values derive from that
                // # ? ? #
                // ? ? ? ?
                // ? ? ? ?
                // # ? ? #
                std::array<glm::vec3, 16> terrain;
                for(size_t x = 0; x < 4; x++) {
                    for(size_t z = 0; z < 4; z++) {
                        // Setup xz spacing
                        terrain[x + 4*z] = glm::vec3(chunk_pos)*Chunk::CHUNK_SIZE + glm::vec3(x*CHUNK_SIZE/3.0f, 0.0f, z*CHUNK_SIZE/3.0f);
                        
                        // Pick free corner y values
                        if((x == 0 || x == 3) && (z == 0 || z == 3)) {
                            terrain[x + 4*z].y += sample(terrain[x + 4*z]);
                        }
                    }
                }

                // Fix remaining control points using euler integration and x edges
                for(size_t x = 0; x < 4; x++) {
                    terrain[x + 1*4].y = terrain[x + 0*4].y + grad(terrain[x + 0*4]).z*Chunk::CHUNK_SIZE/3.0f;
                    terrain[x + 2*4].y = terrain[x + 3*4].y - grad(terrain[x + 3*4]).z*Chunk::CHUNK_SIZE/3.0f;
                }

                // Fix remaining control points using euler integration and z edges
                for(size_t z = 0; z < 4; z++) {
                    terrain[1 + z*4].y = terrain[0 + z*4].y + grad(terrain[0 + z*4]).x*Chunk::CHUNK_SIZE/3.0f;
                    terrain[2 + z*4].y = terrain[3 + z*4].y - grad(terrain[3 + z*4]).x*Chunk::CHUNK_SIZE/3.0f;
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
    
                return std::make_shared<Chunk>(block_shader, terrain_shader, chunk_pos, terrain, textures, vao, vbo);
            } else {
                return std::make_shared<Chunk>(block_shader, terrain_shader, chunk_pos, std::array<glm::vec3, 16>(), textures, 0, 0);
            }
        }
};

/// Represents the blocks in the world and terrain
/// The world is made up of chunk regions
class World final : public mcmodel::Drawable, NonCopyable {
    private:
        // This world is very sparse, so store as a map
        std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>, std::hash<glm::ivec3>> _chunks;

        std::unordered_map<std::string, std::shared_ptr<Particle>> _particles;

        const unsigned int _seed;

        const std::array<GLuint, 2> _terrain_textures;

        const struct {
            const ShaderProgram& block;
            const ShaderProgram& terrain;
        } _shaders;

    public:
        inline World(const unsigned int seed, const ShaderProgram& block_shader, const ShaderProgram& terrain_shader, const std::array<GLuint, 2> terrain_textures) :
            _seed(seed),
            _terrain_textures(terrain_textures),
            _shaders {block_shader, terrain_shader},
            _chunks()
            {}

        virtual inline ~World() = default;

        /// Gets a chunk at a chunk pos or initializes a new one
        inline std::shared_ptr<Chunk> getChunk(const glm::ivec3& chunk_pos) {
            const std::shared_ptr<Chunk> chunk = this->_chunks[chunk_pos];
            if(chunk == nullptr) {
                return this->_chunks[chunk_pos] = Chunk::from(this->_shaders.block, this->_shaders.terrain, chunk_pos, this->_seed, this->_terrain_textures);
            } else {
                return chunk;
            }
        }

        /// Sets the block at a position or removes it if nullptr
        /// Initializes chunk at pos
        inline void setBlock(const glm::ivec3& pos, std::shared_ptr<Block> block = nullptr) {
            this->getChunk(pos/(int) Chunk::CHUNK_SIZE)->setBlock(pos%(int) Chunk::CHUNK_SIZE, block);
        }

        /// Gets the block at a position or nullptr if none
        /// Does *not* initialize chunk if it does not exists
        inline std::shared_ptr<Block> getBlock(const glm::ivec3& pos) const {
            const auto iter = this->_chunks.find(pos/(int) Chunk::CHUNK_SIZE);
            if(iter != this->_chunks.end()) {
                return iter->second->getBlock(pos%(int) Chunk::CHUNK_SIZE);
            } else {
                return nullptr;
            }
        }

        /// Gets the minimum y value for this chunk, only meaningful for bottom chunk of world
        inline float getTerrainHeight(const float x, const float z) const {
            const auto iter = this->_chunks.find({x/(int) Chunk::CHUNK_SIZE, 0, z/(int) Chunk::CHUNK_SIZE});
            if(iter != this->_chunks.end()) {
                return iter->second->getTerrainHeight(glm::fmod(x, Chunk::CHUNK_SIZE), glm::fmod(z, Chunk::CHUNK_SIZE));
            } else {
                return -INFINITY;
            }
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            for(const auto& entry : this->_chunks) {
                if(entry.second != nullptr) {
                    entry.second->draw(ctx);
                }
            }

            for(const auto& entry : this->_particles) {
                entry.second->draw(ctx);
            }
        }

        inline void update(GLfloat deltaTime) {
            // Update particles and entities
            for(auto& entry : this->_particles) {
                entry.second->update(deltaTime);
            }
        }

        inline void add(std::shared_ptr<Particle> particle) {
            this->_particles[particle->getUUID()] = particle;
        }

        inline void remove(std::shared_ptr<Particle> particle) {
            this->_particles.erase(particle->getUUID());
        }

        inline void remove(const Particle& particle) {
            this->_particles.erase(particle.getUUID());
        }
};

#endif