#ifndef SKYBOX_HPP
#define SKYBOX_HPP

/*
 * Skybox.hpp
 * Trin Wasinger - Fall 2025
 * 
 * Skybox renderer based on https://learnopengl.com/Advanced-OpenGL/Cubemaps
 */

#include <array>
#include <string>
#include <memory>
#include <cstdio>
#include <stb_image.h>

#include "mcmodel.hpp"
#include "ShaderProgram.hpp"
#include "NonCopyable.hpp"


class Skybox final : public mcmodel::Drawable, NonCopyable {
    private:
        const ShaderProgram& _shader;
        const GLuint _day_texture, _night_texture;
        GLuint _vao, _vbo;
        static constexpr std::array<GLfloat, 36*3> _POS = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
        };

    public:
        static inline GLuint load_cubemap(const std::array<std::string, 6>& textures) {
            // Load textures
            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
            GLint width, height, channels;
            GLubyte* data;
            
            // Load images from file
            // For cube map, DON'T flip images to match gl coords
            stbi_set_flip_vertically_on_load(false);

            for(GLuint n = 0; n < 6; n++) {
                if((data = stbi_load(textures[n].c_str(), &width, &height, &channels, 0))) {
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + n, 0, channels == 4 ? GL_RGBA : GL_RGB, width, height, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
                } else {
                    fprintf(stderr, "[ERROR]: Failed to load skybox texture \"%s\"\n", textures[n].c_str());
                }
                stbi_image_free(data);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            return texture;
        }

        /// Pass textures +-xyz (NOTE: different order from mcmodel::cube functions)
        inline Skybox(const ShaderProgram& shader, const std::array<std::string, 6>& day_textures, const std::array<std::string, 6>& night_textures) : _shader(shader), _day_texture(Skybox::load_cubemap(day_textures)), _night_texture(Skybox::load_cubemap(night_textures)), _vao(0), _vbo(0) {
            // Load vao
            glGenVertexArrays(1, &this->_vao);
            glBindVertexArray(this->_vao);

            glGenBuffers(1, &this->_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, this->_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Skybox::_POS), Skybox::_POS.data(), GL_STATIC_DRAW);

            const GLuint attrloc = this->_shader.getAttributeLocation("vPos");
            glEnableVertexAttribArray(attrloc);
            glVertexAttribPointer(attrloc, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), (void*) 0);
        }

        inline virtual ~Skybox() override {
            glDeleteTextures(1, &this->_day_texture);
            glDeleteTextures(1, &this->_night_texture);
            glDeleteVertexArrays(1, &this->_vao);
            glDeleteBuffers(1, &this->_vbo);
        }

        inline virtual void draw(glutils::RenderContext& ctx) const override {
            GLint depthFunc;
            glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);

            // Configure
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);

            this->_shader.useProgram();

            // Draw
            glBindVertexArray(this->_vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, this->_night_texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, this->_day_texture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            // Restore
            glDepthMask(GL_TRUE);
            glDepthFunc(depthFunc);
        }
};
#endif