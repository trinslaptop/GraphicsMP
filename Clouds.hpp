#ifndef CLOUDS_HPP
#define CLOUDS_HPP

#include "mcmodel.hpp"
#include "ShaderProgram.hpp"
#include "NonCopyable.hpp"

#include <memory>
#include <array>

class Clouds final : public mcmodel::Drawable, NonCopyable {
    private:
        const ShaderProgram& _shader;
        const GLuint _texture;
        const GLuint _vao;
        const GLuint _buffers[2];

    public:
        Clouds(const ShaderProgram& shader, const GLuint texture, const GLuint vao, const std::array<GLuint, 2> buffers) : _shader(shader), _texture(texture), _vao(vao), _buffers {buffers[0], buffers[1]} {}

        virtual ~Clouds() override {
            glDeleteTextures(1, &this->_texture);
            glDeleteVertexArrays(1, &this->_vao);
            glDeleteBuffers(2, this->_buffers);
        }

    inline virtual void draw(glutils::RenderContext& ctx) const override {
        ctx.bind(this->_shader);
        glBindVertexArray(this->_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->_texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*) nullptr);
    }

    inline static std::shared_ptr<Clouds> from(const ShaderProgram& shader, const GLuint texture) {
        struct Vertex {
            glm::vec3 vPos;
        } vertices[4] = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        };

        const GLushort indices[6] = {0, 1, 2, 0, 2, 3};

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        GLuint buffers[2];
        glGenBuffers(2, buffers);

        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        GLint attrloc;
        if((attrloc = shader.getAttributeLocation(STRC(vPos))) != -1) {
            glEnableVertexAttribArray(attrloc);
            glVertexAttribPointer(attrloc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, vPos)));
        }
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        return std::make_shared<Clouds>(shader, texture, vao, std::array<GLuint, 2> {buffers[0], buffers[1]});
    }
};

#endif