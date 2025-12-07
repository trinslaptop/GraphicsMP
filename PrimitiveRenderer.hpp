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
            const ShaderProgram& line;
            const ShaderProgram& point;
            // const ShaderProgram& sprite;
        } _shaders;

        const GLuint _sprite_texture;

        GLuint _vao;

    public:
        inline PrimitiveRenderer(const ShaderProgram& cube_shader, const ShaderProgram& line_shader, const ShaderProgram& point_shader, const GLuint sprite_texture) : _shaders {cube_shader, line_shader, point_shader}, _sprite_texture(sprite_texture) {
            // Enable gl_PointSize in shaders
            glEnable(GL_PROGRAM_POINT_SIZE);
            // Make an empty VAO
            glGenVertexArrays(1, &this->_vao);
        }

        inline ~PrimitiveRenderer() {
            glDeleteVertexArrays(1, &this->_vao);
        };

        /// Renders a pixel point, intended for debugging only
        inline void point(glutils::RenderContext& ctx, const glm::vec3 pos = glm::vec3(0.0f), const glm::vec3 color = glm::vec3(1.0f), const float size = 1.0f) {
            ctx.bind(this->_shaders.point);
            
            this->_shaders.point.setProgramUniform("pos", pos);
            this->_shaders.point.setProgramUniform("color", color);
            this->_shaders.point.setProgramUniform("size", size*50.0f);

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_POINTS, 0, 1);
        }

        /// Renders a line between two points, intended for debugging only
        inline void line(glutils::RenderContext& ctx, const glm::vec3 pos1 = glm::vec3(0.0f), const glm::vec3 pos2 = glm::vec3(1.0f), const glm::vec3 color = glm::vec3(1.0f)) {
            ctx.bind(this->_shaders.line);
            
            this->_shaders.point.setProgramUniform("pos1", pos1);
            this->_shaders.point.setProgramUniform("pos2", pos2);
            this->_shaders.point.setProgramUniform("color", color);

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_POINTS, 0, 1);
        }

        /// Renders a cube outline, intended for debugging only
        inline void cube(glutils::RenderContext& ctx, const glm::vec3 pos = glm::vec3(0.0f), const glm::vec3 size = glm::vec3(1.0f), const glm::vec3 color = glm::vec3(1.0f)) {
            ctx.bind(this->_shaders.cube);
            
            this->_shaders.point.setProgramUniform("pos", pos);
            this->_shaders.point.setProgramUniform("color", color);
            this->_shaders.point.setProgramUniform("size", size);

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_POINTS, 0, 1);
        }
};

#endif