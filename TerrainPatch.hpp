#ifndef TERRAINPATCH_HPP
#define TERRAINPATCH_HPP

/*
 * TerrainPatch.hpp
 * Trin Wasinger - Fall 2025
 *
 * Creates a GL patch, tessellates it, and uses it to create a curved surface
 * Instead of using Bezier Curves, this uses a single sine/cosine equation that I think looks very nice
 * 
 * Also handles positions and rotations for entities moving along surface
 */

#include <memory>
#include <array>
#include <cmath>
#include <glm/glm.hpp>

#include "mcmodel.hpp"

class TerrainPatch final : public mcmodel::Drawable {
    private:
        const ShaderProgram& _shader;
        const GLuint _vao, _vbo;
        const GLfloat _size;
        const std::array<GLuint, 2> _textures;
  
    public:
        inline TerrainPatch(const ShaderProgram& shader, const GLuint vao, const GLuint vbo, const GLfloat size, const std::array<GLuint, 2> textures) : _shader(shader), _vao(vao), _vbo(vbo), _size(size), _textures(textures) {}
        inline virtual ~TerrainPatch() override {
            glDeleteVertexArrays(1, &this->_vao);
            glDeleteBuffers(1, &this->_vbo);
        }

        inline static std::shared_ptr<TerrainPatch> from(
            const ShaderProgram& shader,
            const GLfloat size,
            const std::array<GLuint, 2> textures
        ) {
            //  (z)
            //   2 ------- 3
            //   |      /  |
            //   |    x    |
            //   |  /      |
            //   0 ------- 1 (x)
            struct Vertex {
                glm::vec3 vPos;
            } vertices[4] = {
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(size, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, size),
                glm::vec3(size, 0.0f, size)
            };

            GLuint vao, vbo;

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            glGenBuffers(1, &vbo);

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            GLint attrloc;
                
            if((attrloc = shader.getAttributeLocation(STRC(vPos))) != -1) {
                glEnableVertexAttribArray(attrloc);
                glVertexAttribPointer(attrloc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, vPos)));
            }

            return std::make_shared<TerrainPatch>(shader, vao, vbo, size, textures);
        }

        // Non-Copyable
        TerrainPatch(const TerrainPatch&) = delete;
        TerrainPatch& operator=(const TerrainPatch&) = delete;

        /// Bind shaders, textures, and uniforms then draw patches
        inline virtual void draw(glutils::RenderContext& ctx) const override {
            glPatchParameteri(GL_PATCH_VERTICES, 4);
            
            this->_shader.useProgram();
            ctx.bind(this->_shader);
            this->_shader.setProgramUniform("size", this->_size);
            this->_shader.setProgramUniform("tint", glm::vec4(0.19f, 0.5f, 0.0f, 1.0f));

            for(size_t i = 0; i < sizeof(this->_textures)/sizeof(this->_textures[0]); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, this->_textures[i]);
            }

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_PATCHES, 0, 4);

            // Reset
            this->_shader.setProgramUniform("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }

        /// Get the computed/rendered y value of the terrain at a given x, z
        inline GLfloat getTerrainHeight(const GLfloat x, const GLfloat z) const {
            // Same formula for y as in shaders/terrain.te.glsl
            return 3*std::pow(std::sin(0.0005*(x*x + z*z)), 2) + std::pow(std::sin(0.1*x), 2) + 0.0625;
        }

        /// Gets the position of the terrain's surface at a given x, z
        inline glm::vec3 getTerrainPosition(const GLfloat x, const GLfloat z) const {
            // Same formula for y as in shaders/terrain.te.glsl
            return glm::vec3(x, this->getTerrainHeight(x, z), z);
        }

        /// Gets the integer position of the terrain's surface at a given x, z grid position
        inline glm::ivec3 getTerrainPosition(const GLint x, const GLint z) const {
            // Same formula for y as in shaders/terrain.te.glsl
            return glm::ivec3(x, this->getTerrainHeight(x + 0.5, z + 0.5), z);
        }

        /// Get a vector pointing away from the terrain at a given x, z
        inline glm::vec3 getTerrainNormal(const GLfloat x, const GLfloat z) const {
            // Same formula as fNormal in shaders/terrain.te.glsl
            return glm::normalize(glm::vec3(
                0.006*x*std::sin(0.0005*(x*x + z*z))*std::cos(0.0005*(x*x + z*z)) + 0.2*std::sin(0.1*x)*std::cos(0.1*x), // partial x
                -1.0,
                0.006*z*std::sin(0.0005*(x*x + z*z))*std::cos(0.0005*(x*x + z*z)) // partial z
            ));
        }
};

#endif