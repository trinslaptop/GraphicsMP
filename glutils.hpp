#ifndef GLUTILS_HPP
#define GLUTILS_HPP

/*
 * glutils.hpp
 * Trin Wasinger - Fall 2025
 *
 * Assorted stuff to make working with OpenGL easier
 */

#include <string>
#include <vector>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

#include <stb_image.h>

#include <functional>

#include <cstdlib>
#include <ctime>


#ifdef CSCI441_USE_GLEW
    #include <GL/glew.h>
#else
    #include <glad/gl.h>
#endif

#include "ShaderProgram.hpp"

#define _STRC(x) #x
/// Stringify macro, useful for attribute/uniform names
#define STRC(x) _STRC(x)

namespace glutils {
    inline static constexpr GLfloat PI = glm::pi<float>();

    /// Get a random integer in [min, max)
    inline int randi(const int min, const int max) {
        return std::rand() % (max - min) + min;
    }

    /// Get a random float in [0.0, 1.0)
    inline float randf() {
        return (float)(std::rand()) / (float)(RAND_MAX + 1.0f);
    }

    /// Sets the RNG seed to the given value or one plus the last one if argument is absent and returns
    /// the new seed, also cycles the generator a few times
    inline unsigned int srandn(const unsigned int value = 0) {
        static unsigned int seed = time(NULL);
        seed = value ? value : seed + 1;
        std::srand(seed);
        
        for(size_t i = 0; i < 16; i++) std::rand();
        
        return seed;
    }
    
    /// Loads, stores, and releases textures automatically
    /// Repeatedly calling `load()` with the same path (same literally, not logically, "./a.png" != "a.png") returns cached result
    class TextureManager final {
        private:
            std::map<const std::string, GLuint> _textures;

        public:
            TextureManager() = default;
            inline ~TextureManager() {
                for(const auto& texture : this->_textures) {
                    // Release textures
                    glDeleteTextures(1, &texture.second);
                }
            };

            // Non-Copyable
            TextureManager(const TextureManager&) = delete;
            TextureManager& operator=(const TextureManager&) = delete;

            // Retrieves texture from cache or loads it
            inline GLuint load(const std::string path) {
                // Try find existing
                const std::map<std::string, GLuint>::iterator iter = this->_textures.find(path);
                if(iter != this->_textures.end()) {
                    return iter->second;
                }
                
                // Load image from file
                // Flip image to match gl coords
                GLuint handle = 0;
                GLint width, height, channels;
                GLubyte* data;
                
                stbi_set_flip_vertically_on_load(true);

                if((data = stbi_load(path.c_str(), &width, &height, &channels, 0))) {
                    const GLint STORAGE_TYPE = (channels == 4 ? GL_RGBA : GL_RGB);

                    glGenTextures(1, &handle);
                    glBindTexture(GL_TEXTURE_2D, handle);

                    // Set texture parameters
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                    // Transfer image data to the GPU
                    glTexImage2D(GL_TEXTURE_2D, 0, STORAGE_TYPE, width, height, 0, STORAGE_TYPE, GL_UNSIGNED_BYTE, data);

                    // Release image memory from CPU - it now lives on the GPU
                    stbi_image_free(data);
                } else {
                    // Load failed
                    fprintf(stderr, "[ERROR]: Could not load texture map \"%s\"\n", path.c_str());
                    return 1;
                }
                
                this->_textures[path] = handle;
                return handle;
            }
    };

    /// A class to manage MVP matrix and stacking transforms
    class RenderContext final {
        private:
            std::vector<glm::mat4> _transformationStack;
            glm::mat4 _modelMatrix, _viewMatrix, _projectionMatrix;
            GLuint _shader;
            GLint _modelMatrixLocation, _normalMatrixLocation;
            
            inline void _updateShader() const {
                if(this->_shader) {
                    glProgramUniformMatrix4fv(this->_shader, this->_modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(this->_modelMatrix));
                    glProgramUniformMatrix3fv(this->_shader, this->_normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(this->_modelMatrix)))));
                }
            }

        public:
            inline RenderContext(const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix) : _viewMatrix(viewMatrix), _projectionMatrix(projectionMatrix), _transformationStack {glm::mat4(1.0)}, _modelMatrix(1.0), _shader(0), _modelMatrixLocation(0), _normalMatrixLocation(0) {}

            // Non-Copyable
            RenderContext(const RenderContext&) = delete;
            RenderContext& operator=(const RenderContext&) = delete;

            /// Set which shader this context should update
            inline void bind(ShaderProgram& shader, const char* modelMatrixUniformName = "modelMatrix", const char* vpMatrixUniformName = "vpMatrix", const char* normalMatrixUniformName = "normalMatrix") {
                this->_shader = shader.getShaderProgramHandle();
                shader.setProgramUniform(vpMatrixUniformName, this->_projectionMatrix * this->_viewMatrix);
                this->_modelMatrixLocation = shader.getUniformLocation(modelMatrixUniformName);
                this->_normalMatrixLocation = shader.getUniformLocation(normalMatrixUniformName);
                this->_updateShader();
            }

            /// Add a new model transform
            inline void pushTransformation(const glm::mat4& transformation) {
                this->_transformationStack.emplace_back(transformation);
                this->_modelMatrix *= transformation;
                this->_updateShader();
            }
            
            /// Revert last transform
            inline void popTransformation()  {
                if(this->_transformationStack.size() > 1) {
                    this->_transformationStack.pop_back();

                    this->_modelMatrix = glm::mat4(1.0f);
                    for(const auto& tMtx : this->_transformationStack) {
                        this->_modelMatrix *= tMtx;
                    }
                    this->_updateShader();
                }
            }

            /// Clear all transforms
            inline void resetTransformation()  {
                this->_transformationStack = {glm::mat4(1.0)};
                this->_updateShader();
            }

            inline ~RenderContext() {
                this->resetTransformation();
            }
    };
}


#endif