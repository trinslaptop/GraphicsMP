#ifndef MCMODEL_HPP
#define MCMODEL_HPP

/*
 * mcmodel.hpp
 * Trin Wasinger - Fall 2025
 *
 * A set of functions and classes for drawing simple textured cubes and faces akin to Minecraft
 */

#include "glutils.hpp"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <array>

#include "ShaderProgram.hpp"

namespace mcmodel {
    /// Basic interface for something that can be drawn
    class Drawable {
        public:
            Drawable() = default;
            virtual ~Drawable() = default;
            virtual void draw(glutils::RenderContext& ctx) const = 0;
    };

    /// A textured face defined by 4 points, should be rectangular
    /// First texture is the diffuse color (and scaled down ambient) and may be cutout (but not translucent)
    /// Second texture is the specular color (alpha chanel*32 is the shininess) for a dull object, use dull.png, and for a full white glint, use shiny.png
    class TexturedFace final : public Drawable {
        private:
            GLuint _vao;
            GLuint _buffers[2];
            GLuint _textures[2];

            // Non-Copyable
            TexturedFace(const TexturedFace&) = delete;
            TexturedFace& operator=(const TexturedFace&) = delete;

        public:
            inline TexturedFace(const GLuint vao, const std::array<GLuint, 2> buffers, const std::array<GLuint, 2> textures) : _vao(vao), _buffers {buffers[0], buffers[1]}, _textures {textures[0], textures[1]} {}
            
            inline virtual ~TexturedFace() override {
                glDeleteVertexArrays(1, &this->_vao);
                glDeleteBuffers(sizeof(this->_buffers)/sizeof(this->_buffers[0]), this->_buffers);
            }
        
            inline virtual void draw(glutils::RenderContext& ctx) const override {
                for(size_t i = 0; i < sizeof(this->_textures)/sizeof(this->_textures[0]); i++) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, this->_textures[i]);
                }

                glBindVertexArray(this->_vao);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*) nullptr);
            }

            inline static std::shared_ptr<Drawable> from(
                const ShaderProgram& shader,
                const std::array<GLuint, 2> textures,
                const std::array<glm::vec3, 4> pos,
                const glm::vec2 texCoord, // texture offset
                const glm::vec2 texSpan, // how much texture to cover
                const bool flipST = false // Use this to rotate textures 90 degrees, don't change order of pos or else normals get reversed
            ) {
                glm::vec3 normal = glm::normalize(glm::cross(pos[1] - pos[0], pos[2] - pos[0]));

                //  (t)
                //   3 ------- 2
                //   |      /  |
                //   |    x    |
                //   |  /      |
                //   0 ------- 1 (s)
                struct Vertex {
                    glm::vec3 vPos;
                    glm::vec3 vNormal;
                    glm::vec2 vTexCoord;
                } vertices[4] = {
                    {pos[0], normal, texCoord},
                    {pos[1], normal, flipST ? glm::vec2 {texCoord.s, texCoord.t + texSpan.t} : glm::vec2 {texCoord.s + texSpan.s, texCoord.t}},
                    {pos[2], normal, texCoord + texSpan},
                    {pos[3], normal, flipST ? glm::vec2 {texCoord.s + texSpan.s, texCoord.t} : glm::vec2 {texCoord.s, texCoord.t + texSpan.t}}
                };

                const GLushort indices[6] = {0, 1, 2, 0, 2, 3};

                GLuint vao;
                GLuint buffers[2];

                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);

                glGenBuffers(sizeof(buffers)/sizeof(buffers[0]), buffers);
                
                glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

                GLint attrloc;
                
                if((attrloc = shader.getAttributeLocation(STRC(vPos))) != -1) {
                    glEnableVertexAttribArray(attrloc);
                    glVertexAttribPointer(attrloc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, vPos)));
                }

                if((attrloc = shader.getAttributeLocation(STRC(vNormal))) != -1) {
                    glEnableVertexAttribArray(attrloc);
                    glVertexAttribPointer(attrloc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, vNormal)));
                }

                if((attrloc = shader.getAttributeLocation(STRC(vTexCoord))) != -1) {
                    glEnableVertexAttribArray(attrloc);
                    glVertexAttribPointer(attrloc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, vTexCoord)));
                }
                
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

                return std::make_shared<TexturedFace>(vao, std::array<GLuint, 2> {buffers[0], buffers[1]}, textures);
            }
    };

    /// Allows using a std::function of appropriate type as a Drawable
    class Lambda final : public Drawable {
        private:
            std::function<void(glutils::RenderContext& ctx)> _f;

            // Non-Copyable
            Lambda(const Lambda&) = delete;
            Lambda& operator=(const Lambda&) = delete;
        public:
            inline Lambda(std::function<void(glutils::RenderContext& ctx)> f) : _f(f) {}
            inline virtual ~Lambda() = default;
            inline virtual void draw(glutils::RenderContext& ctx) const override {
                this->_f(ctx);
            }
    };
    
    /// Groups together zero or more other drawables and applies a set of transformations to them
    class Group final : public Drawable {
        private:
            const std::vector<std::shared_ptr<Drawable>> _children;

            // Non-Copyable
            Group(const Group&) = delete;
            Group& operator=(const Group&) = delete;
        public:
            glm::vec3 position;
            
            // x, y, z are yaw pitch roll
            glm::vec3 rotation;
            
            glm::vec3 scale;
            
            bool hidden;
            
            inline Group(
                std::initializer_list<std::shared_ptr<Drawable>> children,
                const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
                const glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f),
                const glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f),
                const bool hidden = false
            ) :
                _children(children), 
                position(position),
                rotation(rotation),
                scale(scale),
                hidden(hidden)
            {}

            inline virtual ~Group() = default;

            inline virtual void draw(glutils::RenderContext& ctx) const override {
                if(!hidden) {
                    ctx.pushTransformation(glm::scale(
                        glm::yawPitchRoll(this->rotation.x, this->rotation.y, this->rotation.z)*glm::translate(glm::mat4(1.0f), this->position),
                        this->scale
                    ));

                    for(const auto& child : this->_children) {
                        child->draw(ctx);
                    }

                    ctx.popTransformation();
                }
            }
    };

    /// Wrap a drawable with this to enable face fulling, restores old state when done
    /// Useful for performance gains if you know you won't see the back face or to remove z-fighting when placing two opposite facing faces together 
    inline std::shared_ptr<Drawable> cullface(std::shared_ptr<Drawable> child, const GLenum face = GL_BACK) {
        return std::make_shared<Lambda>([child, face](glutils::RenderContext& ctx) {
            // Get old values
            const GLboolean _enabled = glIsEnabled(GL_CULL_FACE);
            GLint _face;
            glGetIntegerv(GL_CULL_FACE_MODE, &_face);

            // Draw
            glEnable(GL_CULL_FACE);
            glCullFace(face);
            child->draw(ctx);
            
            // Restore old values
            glCullFace(_face);
            if(!_enabled) {
                glDisable(GL_CULL_FACE);
            }
        });
    }

    
    inline std::shared_ptr<Drawable> oscillate(const ShaderProgram& shader, std::shared_ptr<Drawable> child, const GLfloat oscillation = 1.0f) {
        const GLuint handle = shader.getShaderProgramHandle(), attrloc = shader.getUniformLocation("oscillation");
        return std::make_shared<Lambda>([handle, attrloc, child, oscillation](glutils::RenderContext& ctx) {
            glProgramUniform1f(handle, attrloc, oscillation);
            child->draw(ctx);
            glProgramUniform1f(handle, attrloc, 0.0f);
        });
    }

    /// Helper to create a Group, cuts out `std::make_shared<Group>(std::initializer_list<Drawable> {...})`
    inline std::shared_ptr<Drawable> group(std::initializer_list<std::shared_ptr<Drawable>> children, const glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f), const bool hidden = false) {
        return std::make_shared<Group>(children, position, rotation, scale, hidden);
    }

    /// Creates a cube, if 6 texture sets are given, each -+xyz face gets one in that order; otherwise, each face uses the first texture set
    template<size_t NUM_TEXTURES> inline std::shared_ptr<Drawable> cube(const ShaderProgram& shader, const std::array<std::array<GLuint, 2>, NUM_TEXTURES>& textures, const glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f)) {
        static_assert(NUM_TEXTURES == 1 || NUM_TEXTURES == 6, "NUM_TEXTURES must be 1 or 6");
        return std::make_shared<Group>(std::initializer_list<std::shared_ptr<Drawable>> {
            // x
            TexturedFace::from(shader, textures[NUM_TEXTURES == 1 ? 0 : 0], {
                pos + size*glm::vec3(0, 0, 0), pos + size*glm::vec3(0, 0, 1), pos + size*glm::vec3(0, 1, 1), pos + size*glm::vec3(0, 1, 0)
            }, glm::vec2(pos.y, pos.z), glm::vec2(size.y, size.z)),
            TexturedFace::from(shader, textures[NUM_TEXTURES == 1 ? 0 : 1], {
                pos + size*glm::vec3(1, 0, 0), pos + size*glm::vec3(1, 1, 0), pos + size*glm::vec3(1, 1, 1), pos + size*glm::vec3(1, 0, 1)
            }, glm::vec2(pos.y, pos.z), glm::vec2(size.y, size.z), true),
            
            // y
            TexturedFace::from(shader, textures[NUM_TEXTURES == 1 ? 0 : 2], {
                pos + size*glm::vec3(0, 0, 0), pos + size*glm::vec3(1, 0, 0), pos + size*glm::vec3(1, 0, 1), pos + size*glm::vec3(0, 0, 1)
            }, glm::vec2(pos.x, pos.z), glm::vec2(size.x, size.z), true),
            TexturedFace::from(shader, textures[NUM_TEXTURES == 1 ? 0 : 3], {
                pos + size*glm::vec3(0, 1, 0), pos + size*glm::vec3(0, 1, 1), pos + size*glm::vec3(1, 1, 1), pos + size*glm::vec3(1, 1, 0)
            }, glm::vec2(pos.x, pos.z), glm::vec2(size.x, size.z)),
            
            // z
            TexturedFace::from(shader, textures[NUM_TEXTURES == 1 ? 0 : 4], {
                pos + size*glm::vec3(0, 0, 0), pos + size*glm::vec3(0, 1, 0), pos + size*glm::vec3(1, 1, 0), pos + size*glm::vec3(1, 0, 0)
            }, glm::vec2(pos.x, pos.y), glm::vec2(size.x, size.y), true),
            TexturedFace::from(shader, textures[NUM_TEXTURES == 1 ? 0 : 5], {
                pos + size*glm::vec3(0, 0, 1), pos + size*glm::vec3(1, 0, 1), pos + size*glm::vec3(1, 1, 1), pos + size*glm::vec3(0, 1, 1)
            }, glm::vec2(pos.x, pos.y), glm::vec2(size.x, size.y))
        });
    }

    /// Creates a cross of two y aligned planes, good for basic plants
    inline std::shared_ptr<Drawable> cross(const ShaderProgram& shader, const std::array<GLuint, 2>& textures, const glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f), const GLfloat size = 1.0f) {
        return cullface(group({
            TexturedFace::from(shader, textures, {
                pos + size*glm::vec3(0, 0, 0), pos + size*glm::vec3(1, 0, 1), pos + size*glm::vec3(1, 1, 1), pos + size*glm::vec3(0, 1, 0)
            }, glm::vec2(0, 0), glm::vec2(size, size)),
            TexturedFace::from(shader, textures, {
                pos + size*glm::vec3(0, 1, 0), pos + size*glm::vec3(1, 1, 1), pos + size*glm::vec3(1, 0, 1), pos + size*glm::vec3(0, 0, 0)
            }, glm::vec2(0, 0), glm::vec2(size, -size)),
            TexturedFace::from(shader, textures, {
                pos + size*glm::vec3(1, 0, 0), pos + size*glm::vec3(0, 0, 1), pos + size*glm::vec3(0, 1, 1), pos + size*glm::vec3(1, 1, 0)
            }, glm::vec2(0, 0), glm::vec2(size, size)),
            TexturedFace::from(shader, textures, {
                pos + size*glm::vec3(1, 1, 0), pos + size*glm::vec3(0, 1, 1), pos + size*glm::vec3(0, 0, 1), pos + size*glm::vec3(1, 0, 0)
            }, glm::vec2(0, 0), glm::vec2(size, -size))
        }));
    }

    /// Creates a cube and wraps a single texture set around it
    ///      Top   Bottom
    /// Left Front Right  Back
    inline std::shared_ptr<Drawable> wrapped_cube(const ShaderProgram& shader, const std::array<GLuint, 2>& textures, const glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f), const glm::vec2 texScale = glm::vec2(1.0f, 1.0f), const glm::vec2 texOffset = glm::vec2(0.0f, 0.0f)) {
        return std::make_shared<Group>(std::initializer_list<std::shared_ptr<Drawable>> {
            // x
            TexturedFace::from(shader, textures, {
                size*glm::vec3(0, 0, 0), size*glm::vec3(0, 0, 1), size*glm::vec3(0, 1, 1), size*glm::vec3(0, 1, 0)
            }, texOffset + glm::vec2(size.x + size.z + size.x, 0)/texScale, glm::vec2(size.z, size.y)/texScale),
            TexturedFace::from(shader, textures, {
                size*glm::vec3(1, 0, 0), size*glm::vec3(1, 1, 0), size*glm::vec3(1, 1, 1), size*glm::vec3(1, 0, 1)
            }, texOffset + glm::vec2(size.x + size.z, 0)/texScale, glm::vec2(-size.z, size.y)/texScale, true),
            
            // y
            TexturedFace::from(shader, textures, {
                size*glm::vec3(0, 0, 0), size*glm::vec3(1, 0, 0), size*glm::vec3(1, 0, 1), size*glm::vec3(0, 0, 1)
            }, texOffset + glm::vec2(size.x + size.z + size.z, size.y + size.x)/texScale, glm::vec2(-size.z, -size.x)/texScale, true),
            TexturedFace::from(shader, textures, {
                size*glm::vec3(0, 1, 0), size*glm::vec3(0, 1, 1), size*glm::vec3(1, 1, 1), size*glm::vec3(1, 1, 0)
            }, texOffset + glm::vec2(size.x + size.z, size.y + size.x)/texScale, glm::vec2(-size.z, -size.x)/texScale),
            
            // z
            TexturedFace::from(shader, textures, {
                size*glm::vec3(0, 0, 0), size*glm::vec3(0, 1, 0), size*glm::vec3(1, 1, 0), size*glm::vec3(1, 0, 0)
            }, texOffset + glm::vec2(size.x + size.z + size.x, 0)/texScale, glm::vec2(-size.x, size.y)/texScale, true),
            TexturedFace::from(shader, textures, {
                size*glm::vec3(0, 0, 1), size*glm::vec3(1, 0, 1), size*glm::vec3(1, 1, 1), size*glm::vec3(0, 1, 1)
            }, texOffset, glm::vec2(size.x, size.y)/texScale)
        }, -size/2.0f);
    }
}

#endif