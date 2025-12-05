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
        GLuint _texture;
        GLuint _vao, _vbo;
        const GLuint _vploc;
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
        /// Pass textures +-xyz (NOTE: different order from mcmodel::cube functions)
        inline Skybox(const ShaderProgram& shader, const std::array<std::string, 6>& textures) : _shader(shader), _texture(0), _vao(0), _vbo(0), _vploc(shader.getUniformLocation("vpMatrix")) {
            // Load textures
            glGenTextures(1, &this->_texture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, this->_texture);
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
            glDeleteTextures(1, &this->_texture);
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

            // Remove view rotation
            this->_shader.setProgramUniform(this->_vploc, ctx.getProjectionMatrix()*glm::mat4(glm::mat3(ctx.getViewMatrix())));

            // Draw
            glBindVertexArray(this->_vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, this->_texture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            // Restore
            glDepthMask(GL_TRUE);
            glDepthFunc(depthFunc);
        }
};
#endif