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

#include <fstream>
#include <sstream>
#include <ios>
#include <string_view>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

#include <stb_image.h>

#include <functional>

#include <cstdlib>
#include <cstdint>
#include <ctime>


#ifdef CSCI441_USE_GLEW
    #include <GL/glew.h>
#else
    #include <glad/gl.h>
#endif

#include <CSCI441/Camera.hpp>

#include "NonCopyable.hpp"
#include "ShaderProgram.hpp"

#define _STRC(x) #x
/// Stringify macro, useful for attribute/uniform names
#define STRC(x) _STRC(x)

namespace glutils {
    inline static constexpr GLfloat PI = glm::pi<float>();

    namespace {
        /// With %s, snprintf expects a const char* not a std::string, so remap it
        template<typename T> inline auto format_value(T&& t) {
            if constexpr(std::is_same<std::decay_t<T>, std::string>::value) {
                return std::forward<T>(t).c_str();
            } else {
                return std::forward<T>(t);
            }
        }
    }

	/// Format a string using standard printf modifiers like %s and %d
	template<typename... Args> [[gnu::format(printf, 1, 0)]] inline std::string format(const char* format, Args&&... args) {
		const int n = std::snprintf(nullptr, 0, format, format_value(std::forward<Args>(args))...) + 1;
		
		if(n <= 0) {
			throw std::runtime_error("Bad format string");
		}

		std::string buffer(n, '\0');
		std::snprintf(buffer.data(), buffer.size(), format, format_value(std::forward<Args>(args))...);
		buffer.resize(buffer.size() - 1);

		return buffer;
	}

    /// Cubic Bezier Curve formula
    template<typename T> inline T bc(const T p0, const T p1, const T p2, const T p3, const float t) {
        return glm::pow(1 - t, 3)*p0 + 3*glm::pow(1 - t, 2)*t*p1 + 3*(1-t)*glm::pow(t, 2)*p2 + glm::pow(t, 3)*p3;
    }

    /// Safe divide, if denominator is zero then returns zero
    template<typename T> inline T safediv(const T& numerator, const T& denominator) {
        return denominator == T() ? T() : numerator/denominator;
    }

    /// Read a file into a string (not ideal for large files!)
    inline std::string cat(const std::string& path) {
        std::ifstream istream(path);
        istream.exceptions(std::ios_base::badbit);
        std::ostringstream ostream;
        ostream << istream.rdbuf();
        return ostream.str();
    }

    /// Checks is `string` ends with `suffix`
    /// Based on https://stackoverflow.com/a/42844629
    inline bool string_ends_with(std::string_view string, std::string_view suffix) {
        return string.size() >= suffix.size() && string.compare(string.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    /// Checks is `string` starts with `prefix`
    /// Based on https://stackoverflow.com/a/42844629
    inline bool string_starts_with(std::string_view string, std::string_view prefix) {
        return string.size() >= prefix.size() && string.compare(0, prefix.size(), prefix) == 0;
    }

    /// Lists all entries in a directory. If `ext` is not empty, filters for only that extension
    inline std::vector<std::string> ls(const std::string& path, const std::string& ext = "") {
        std::vector<std::string> paths;
        for(const auto& entry : std::filesystem::directory_iterator(path)) {
            if(ext.empty() || string_ends_with(entry.path().string(), "." + ext)) {
                paths.push_back(entry.path().string());
            }
        }
        return paths;
    }

    // Gets a RGB color vector from hex
    inline glm::vec3 hex3(const uint32_t color) {
        return glm::vec3((color >> 16) & 255, (color >> 8) & 255, color & 255)/255.0f;
    }

    // Gets a RGBA color vector from hex
    inline glm::vec4 hex4(const uint32_t color) {
        return glm::vec4((color >> 24) & 255, (color >> 16) & 255, (color >> 8) & 255, color & 255)/255.0f;
    }
    
    /// Loads, stores, and releases textures automatically
    /// Repeatedly calling `load()` with the same path (same literally, not logically, "./a.png" != "a.png") returns cached result
    class TextureManager final : NonCopyable {
        private:
            std::map<std::string, GLuint> _textures;
        public:
            const GLuint DEFAULT, DULL, SHINY; // Make default.png texture handle 1, any textures that fail to load will fallback to this, NOTE: these must be declared after `_textures` so that they can be initialized using `load()`

            TextureManager() : DEFAULT(load("assets/textures/default.png")), DULL(load("assets/textures/dull.png")), SHINY(load("assets/textures/shiny.png")) {}

            inline ~TextureManager() {
                for(const auto& texture : this->_textures) {
                    // Release textures
                    glDeleteTextures(1, &texture.second);
                }
            };

            // Retrieves texture from cache or loads it from a bytes
            inline GLuint load(const std::string& name, const std::vector<unsigned char>& bytes) {
                // Try find existing
                std::map<std::string, GLuint>::iterator iter = this->_textures.find(name);
                if(iter != this->_textures.end()) {
                    return iter->second;
                }
                
                // Load image from bytes
                GLuint handle = 0;
                GLint width, height, channels;
                GLubyte* data;
                
                // Flip image to match gl coords
                stbi_set_flip_vertically_on_load(true);

                if((data = stbi_load_from_memory(bytes.data(), bytes.size(), &width, &height, &channels, 0))) {
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
                    fprintf(stderr, "[ERROR]: Could not load texture map \"%s\"\n", name.c_str());
                    return 1;
                }
                
                this->_textures[name] = handle;
                return handle;
            }

            // Retrieves texture from cache or loads it from a file
            inline GLuint load(const std::string& path) {
                // Try find existing
                std::map<std::string, GLuint>::iterator iter = this->_textures.find(path);
                if(iter != this->_textures.end()) {
                    return iter->second;
                }
                
                // Load image from file
                GLuint handle = 0;
                GLint width, height, channels;
                GLubyte* data;
                
                // Flip image to match gl coords
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

    /// Gets the handle for the currently bound shader or 0 if none
    inline GLint get_shader() {
        GLint shader;
        glGetIntegerv(GL_CURRENT_PROGRAM, &shader);
        return shader;
    }

    /// A utility for rendering common shapes (points, lines, cubes, 2D rectangles, and textured sprites)
    /// Most work is done by geometry shaders
    class PrimitiveRenderer final : NonCopyable {
        private:
            const struct {
                const ShaderProgram& cube;
                const ShaderProgram& line;
                const ShaderProgram& point;
                const ShaderProgram& rect;
                const ShaderProgram& sprite;
            } _shaders;

            const GLuint _sprite_texture, _dull_texture;

            GLuint _vao;

        public:
            inline PrimitiveRenderer(const ShaderProgram& cube_shader, const ShaderProgram& line_shader, const ShaderProgram& point_shader, const ShaderProgram& rect_shader, const ShaderProgram& sprite_shader, const GLuint sprite_texture, const GLuint dull_texture) : _shaders {cube_shader, line_shader, point_shader, rect_shader, sprite_shader}, _sprite_texture(sprite_texture), _dull_texture(dull_texture) {
                // Enable gl_PointSize in shaders
                glEnable(GL_PROGRAM_POINT_SIZE);

                // Make an empty VAO
                glGenVertexArrays(1, &this->_vao);
            }

            inline ~PrimitiveRenderer() {
                glDeleteVertexArrays(1, &this->_vao);
            };

            /// Renders a pixel point, intended for debugging only
            inline void point(const glm::vec3 pos = glm::vec3(0.0f), const glm::vec3 color = glm::vec3(1.0f), const float size = 1.0f) const {
                const GLint shader = get_shader();
                this->_shaders.point.useProgram();
                
                this->_shaders.point.setProgramUniform("pos", pos);
                this->_shaders.point.setProgramUniform("color", color);
                this->_shaders.point.setProgramUniform("size", size*50.0f);

                glBindVertexArray(this->_vao);
                glDrawArrays(GL_POINTS, 0, 1);

                glUseProgram(shader);
            }

            /// Renders a line between two points, intended for debugging only
            inline void line(const glm::vec3 pos1 = glm::vec3(0.0f), const glm::vec3 pos2 = glm::vec3(1.0f), const glm::vec3 color = glm::vec3(1.0f)) const {
                const GLint shader = get_shader();
                this->_shaders.line.useProgram();
                
                this->_shaders.line.setProgramUniform("pos1", pos1);
                this->_shaders.line.setProgramUniform("pos2", pos2);
                this->_shaders.line.setProgramUniform("color", color);

                glBindVertexArray(this->_vao);
                glDrawArrays(GL_POINTS, 0, 1);
                glUseProgram(shader);
            }

            /// Renders a cube outline, intended for debugging only
            inline void cube(const glm::vec3 pos = glm::vec3(0.0f), const glm::vec3 size = glm::vec3(1.0f), const glm::vec3 color = glm::vec3(1.0f), const glm::bvec3 centered = glm::bvec3(false, false, false)) const {
                const GLint shader = get_shader();
                this->_shaders.cube.useProgram();
                
                this->_shaders.cube.setProgramUniform("pos", pos - glm::vec3(centered)*size/2.0f);
                this->_shaders.cube.setProgramUniform("color", color);
                this->_shaders.cube.setProgramUniform("size", size);

                glBindVertexArray(this->_vao);
                glDrawArrays(GL_POINTS, 0, 1);
                glUseProgram(shader);
            }

            /// Renders a colored rectangle on the UI
            inline void rect(const glm::vec2 pos = glm::vec2(0.0f), const glm::vec2 size = glm::vec2(1.0f), const glm::vec4 color = glm::vec4(1.0f), const glm::bvec2 centered = glm::bvec2(false, false)) const {
                const GLint shader = get_shader();
                this->_shaders.rect.useProgram();

                this->_shaders.rect.setProgramUniform("pos", pos - glm::vec2(centered)*size);
                this->_shaders.rect.setProgramUniform("color", color);
                this->_shaders.rect.setProgramUniform("size", size);

                glBindVertexArray(this->_vao);
                glDrawArrays(GL_POINTS, 0, 1);
                glUseProgram(shader);
            }

            /// Triggers the rendering pipeline with an empty VAO, use with shaders that take no attributes
            inline void draw() const {
                glBindVertexArray(this->_vao);
                glDrawArrays(GL_POINTS, 0, 1);
            }

            enum SpriteMode {
                #include "shaders/primitives/SpriteMode.glsl"
            };

            /// Returns the area covered by the sprites, useful for using rect() to fill below text
            /// `lit` only applies to PARTICLE mode
            inline glm::vec2 sprite(const std::string& sprites, const glm::vec3 pos, const glm::vec3 color = glm::vec3(1.0f), const float size = 1.0f, const SpriteMode mode = SpriteMode::UI_ANCHOR_CORNER, const bool lit = false) const {
                const GLint shader = get_shader();
                this->_shaders.sprite.useProgram();

                glm::vec2 offset = glm::vec2(0.0, 0.0);

                this->_shaders.sprite.setProgramUniform("size", size);
                this->_shaders.sprite.setProgramUniform("mode", mode);
                
                this->_shaders.sprite.setProgramUniform("tint", glm::vec4(color, 1.0f));
                this->_shaders.sprite.setProgramUniform("lit", mode == SpriteMode::PARTICLE && lit);
                this->_shaders.sprite.setProgramUniform("frameCount", (GLuint) 1);
                this->_shaders.sprite.setProgramUniform("frameTime", 1.0f);

                glBindVertexArray(this->_vao);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, this->_sprite_texture);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, this->_dull_texture);

                for(const char c : sprites) {
                    switch(c) {
                        case '\t':
                            offset.x += 4*size;
                            break;
                        case '\n':
                            offset.y -= size;
                            offset.x = 0;
                            break;
                        default:
                            if(mode != SpriteMode::HIDDEN) {
                                this->_shaders.sprite.setProgramUniform("sprite", c);
                                this->_shaders.sprite.setProgramUniform("pos", pos + glm::vec3(offset, 0.0));
                                glDrawArrays(GL_POINTS, 0, 1);
                            }
                            offset.x += size;
                            break;
                    }
                }

                glUseProgram(shader);
                return offset + size;
            }
    };

    /// A class to manage stacking transforms
    /// NOTE: VP matrices have been moved to a UBO
    class RenderContext final : NonCopyable {
        private:
            const PrimitiveRenderer& _pr;
            const bool _debug;
            std::vector<glm::mat4> _transformationStack;
            glm::mat4 _modelMatrix;
            GLuint _shader;
            GLint _modelMatrixLocation, _normalMatrixLocation;
            
            inline void _updateShader() const {
                if(this->_shader) {
                    glProgramUniformMatrix4fv(this->_shader, this->_modelMatrixLocation, 1, GL_FALSE, glm::value_ptr(this->_modelMatrix));
                    glProgramUniformMatrix3fv(this->_shader, this->_normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(glm::mat3(glm::transpose(glm::inverse(this->_modelMatrix)))));
                }
            }

        public:
            inline RenderContext(const PrimitiveRenderer& pr, const bool debug) : _transformationStack {glm::mat4(1.0)}, _modelMatrix(1.0), _shader(0), _modelMatrixLocation(0), _normalMatrixLocation(0), _pr(pr), _debug(debug) {}
                        
            /// Set which shader this context should update
            inline void bind(const ShaderProgram& shader, const char* modelMatrixUniformName = "modelMatrix", const char* normalMatrixUniformName = "normalMatrix") {
                this->_shader = shader.getShaderProgramHandle();
                shader.useProgram();
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

            inline const PrimitiveRenderer& getPrimitiveRenderer() const {
                return this->_pr;
            }

            inline bool debug() const {
                return this->_debug;
            }
    };
}


#endif