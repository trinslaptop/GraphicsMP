#ifndef TERRAINPATCH_HPP
#define TERRAINPATCH_HPP

#include <memory>
#include <array>

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

        inline static std::shared_ptr<Drawable> from(
            const ShaderProgram& shader,
            const GLfloat size,
            const std::array<GLuint, 2> textures
        ) {
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

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            glPatchParameteri(GL_PATCH_VERTICES, 4);
            
            this->_shader.useProgram();
            ctx.bind(this->_shader);
            this->_shader.setProgramUniform("size", this->_size);

            for(size_t i = 0; i < sizeof(this->_textures)/sizeof(this->_textures[0]); i++) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, this->_textures[i]);
            }

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_PATCHES, 0, 4);
        }
};

#endif