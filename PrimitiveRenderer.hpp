#ifndef PRIMITIVERENDERER_HPP
#define PRIMITIVERENDERER_HPP

#include <string>

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
            const ShaderProgram& sprite;
        } _shaders;

        const GLuint _sprite_texture;

        GLuint _vao;

    public:
        inline PrimitiveRenderer(const ShaderProgram& cube_shader, const ShaderProgram& line_shader, const ShaderProgram& point_shader, const ShaderProgram& sprite_shader, const GLuint sprite_texture) : _shaders {cube_shader, line_shader, point_shader, sprite_shader}, _sprite_texture(sprite_texture) {
            // Enable gl_PointSize in shaders
            glEnable(GL_PROGRAM_POINT_SIZE);

            // Make an empty VAO
            glGenVertexArrays(1, &this->_vao);
        }

        inline ~PrimitiveRenderer() {
            glDeleteVertexArrays(1, &this->_vao);
        };

        /// Renders a pixel point, intended for debugging only
        inline void point(const glm::vec3 pos = glm::vec3(0.0f), const glm::vec3 color = glm::vec3(1.0f), const float size = 1.0f) {
            this->_shaders.point.useProgram();
            
            this->_shaders.point.setProgramUniform("pos", pos);
            this->_shaders.point.setProgramUniform("color", color);
            this->_shaders.point.setProgramUniform("size", size*50.0f);

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_POINTS, 0, 1);
        }

        /// Renders a line between two points, intended for debugging only
        inline void line(const glm::vec3 pos1 = glm::vec3(0.0f), const glm::vec3 pos2 = glm::vec3(1.0f), const glm::vec3 color = glm::vec3(1.0f)) {
            this->_shaders.line.useProgram();
            
            this->_shaders.line.setProgramUniform("pos1", pos1);
            this->_shaders.line.setProgramUniform("pos2", pos2);
            this->_shaders.line.setProgramUniform("color", color);

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_POINTS, 0, 1);
        }

        /// Renders a cube outline, intended for debugging only
        inline void cube(const glm::vec3 pos = glm::vec3(0.0f), const glm::vec3 size = glm::vec3(1.0f), const glm::vec3 color = glm::vec3(1.0f), const glm::bvec3 centered = glm::bvec3(false, false, false)) {
            this->_shaders.cube.useProgram();
            
            this->_shaders.cube.setProgramUniform("pos", pos - glm::vec3(centered)*size/2.0f);
            this->_shaders.cube.setProgramUniform("color", color);
            this->_shaders.cube.setProgramUniform("size", size);

            glBindVertexArray(this->_vao);
            glDrawArrays(GL_POINTS, 0, 1);
        }

        enum SpriteMode {
            #include "shaders/primitives/SpriteMode.glsl"
        };

        inline void sprite(const std::string& sprites, const glm::vec3 pos, const glm::vec3 color = glm::vec3(1.0f), const float size = 1.0f, const SpriteMode mode = SpriteMode::UI_ANCHOR_CORNER) {
            this->_shaders.sprite.useProgram();

            glm::vec2 offset = glm::vec2(0.0, 0.0);

            this->_shaders.sprite.setProgramUniform("size", 2*size);
            this->_shaders.sprite.setProgramUniform("mode", mode);
            
            this->_shaders.sprite.setProgramUniform("tint", glm::vec4(color, 1.0f));
            this->_shaders.sprite.setProgramUniform("lit", false);
            this->_shaders.sprite.setProgramUniform("frameCount", (GLuint) 1);
            this->_shaders.sprite.setProgramUniform("frameTime", 1.0f);

            glBindVertexArray(this->_vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, this->_sprite_texture);

            for(const char c : sprites) {
                switch(c) {
                    case '\t':
                        offset.x += 8*size;
                        break;
                    case '\n':
                        offset.y -= 2*size;
                        offset.x = 0;
                        break;
                    default:
                        this->_shaders.sprite.setProgramUniform("sprite", c);
                        this->_shaders.sprite.setProgramUniform("pos", pos + glm::vec3(offset, 0.0));
                        glDrawArrays(GL_POINTS, 0, 1);
                        offset.x += 2*size;
                        break;
                }
            }
        }
};

#endif