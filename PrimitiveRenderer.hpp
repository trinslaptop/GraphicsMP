#ifndef PRIMITIVERENDERER_HPP
#define PRIMITIVERENDERER_HPP

#include "NonCopyable.hpp"
#include "ShaderProgram.hpp"
#include "glutils.hpp"

#include <glm/glm.hpp>

class PrimitiveRenderer final : NonCopyable {
    private:
        const struct {
            const ShaderProgram& cube;
            const ShaderProgram& point;
            // const ShaderProgram& sprite;
        } _shaders;

        const GLuint _sprite_texture;

        GLuint _vao;

    public:
        inline PrimitiveRenderer(const ShaderProgram& cube_shader, const ShaderProgram& point_shader, const GLuint sprite_texture) : _shaders {cube_shader, point_shader}, _sprite_texture(sprite_texture) {
            glEnable(GL_PROGRAM_POINT_SIZE);
            glGenVertexArrays(1, &this->_vao);
        }

        inline ~PrimitiveRenderer() {
            glDeleteVertexArrays(1, &this->_vao);
        };

        inline void point(glutils::RenderContext& ctx, const glm::vec3 pos = glm::vec3(0.0f), const glm::vec3 color = glm::vec3(1.0f), const float size = 1.0f) {
            ctx.bind(this->_shaders.point);
            
            this->_shaders.point.setProgramUniform("pos", pos);
            this->_shaders.point.setProgramUniform("color", color);
            this->_shaders.point.setProgramUniform("size", size*50.0f);

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_POINTS, 0, 1);
        }
};

#endif