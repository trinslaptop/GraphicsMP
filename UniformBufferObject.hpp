#ifndef UNIFORMBUFFEROBJECT_HPP
#define UNIFORMBUFFEROBJECT_HPP

#include <cstddef>
#include <map>

#include "glutils.hpp"
#include "ShaderProgram.hpp"

#include <glm/gtc/type_ptr.hpp>

/// Wraps an OpenGL UBO to allow sharing shader uniforms. Bind once and it's set.
class UniformBufferObject final {
    private:
        GLuint _buffer, _binding;
        std::map<std::string, GLint> _offsets;

        inline void _setUniform(const char* name, const size_t size, const void* data) const {
            const auto iter = this->_offsets.find(name);
            if(iter != this->_offsets.end()) {
                glBindBuffer(GL_UNIFORM_BUFFER, this->_buffer);
                glBufferSubData(GL_UNIFORM_BUFFER, iter->second, size, data);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            } else {
                fprintf(stderr, "[ERROR]: Unknown uniform \"%s\" for UBO (try binding a shader first?)\n", name);
            }
        }
    public:
        inline UniformBufferObject(const GLuint binding) : _buffer(0), _binding(binding) {}

        inline virtual ~UniformBufferObject() {
            glDeleteBuffers(1, &this->_buffer);
        }

        inline void bindShaderBlock(const ShaderProgram& shader, const char* name) {
            const GLint blockIndex = shader.getUniformBlockIndex(name);
            if(blockIndex != GL_INVALID_INDEX) {
                // Initialize buffer
                if(this->_buffer == 0) {
                    GLint size;
                    glGetActiveUniformBlockiv(shader.getShaderProgramHandle(), blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &size);

                    glGenBuffers(1, &this->_buffer);
                    glBindBuffer(GL_UNIFORM_BUFFER, this->_buffer);
                    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
                    glBindBufferBase(GL_UNIFORM_BUFFER, this->_binding, this->_buffer);
                    glBindBuffer(GL_UNIFORM_BUFFER, 0);
                }

                // Bind block
                shader.setUniformBlockBinding(name, this->_binding);

                // Get count of uniforms in block
                GLint count;
                glGetActiveUniformBlockiv(shader.getShaderProgramHandle(), blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &count);

                // Get indices of uniforms in block
                std::unique_ptr<GLuint[]> indices = std::make_unique<GLuint[]>(count);
                glGetActiveUniformBlockiv(shader.getShaderProgramHandle(), blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, reinterpret_cast<GLint*>(indices.get()));

                // Get max name length
                GLint max_length;
                glGetProgramiv(shader.getShaderProgramHandle(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);

                // Loop over members and find offsets
                std::unique_ptr<GLchar[]> buffer = std::make_unique<GLchar[]>(max_length);
                GLsizei length;
                for(GLint i = 0; i < count; i++) {
                    glGetActiveUniformName(shader.getShaderProgramHandle(), indices[i], max_length, &length, buffer.get());

                    const std::string name(buffer.get(), length);
                    if(this->_offsets.find(name) == this->_offsets.end()) {
                        GLint offset;
                        glGetActiveUniformsiv(shader.getShaderProgramHandle(), 1, &indices[i], GL_UNIFORM_OFFSET, &offset);

                        this->_offsets[name] = offset;
                    }
                }
            }
        }

        [[maybe_unused]] inline void setUniform(const char* name, const GLfloat value) {
            this->_setUniform(name, sizeof(GLfloat), &value);
        }

        [[maybe_unused]] inline void setUniform(const char* name, const GLint value) {
            this->_setUniform(name, sizeof(GLint), &value);
        }

        [[maybe_unused]] inline void setUniform(const char* name, const GLuint value) {
            this->_setUniform(name, sizeof(GLuint), &value);
        }

        template<typename T> [[maybe_unused]] inline void setUniform(const char* name, const T value) {
            this->_setUniform(name, sizeof(T), glm::value_ptr(value));
        }
};

#endif