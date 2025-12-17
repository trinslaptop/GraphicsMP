#ifndef SHADOWMAP_HPP
#define SHADOWMAP_HPP

#include "NonCopyable.hpp"

#include "glutils.hpp"
#include "UniformBufferObject.hpp"
#include "mcmodel.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

/// Handles rendering shadows to a FBO
/// NOTE: due to legacy code, the shadow rendering is adapted to work with modified existing shaders instead of new ones!
/// Based on https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
class ShadowMap final : NonCopyable {
    private:
        GLuint _fbo, _depthmap;
        const glm::mat4 _projection;
    public:
        const GLuint WIDTH, HEIGHT;
        inline ShadowMap(const GLuint width = 1024, const GLuint height = 1024, const float near_plane = 0.1f, const float far_plane = 100.0f) : WIDTH(width), HEIGHT(height), _projection(glm::ortho(-32.0f, 32.0f, -32.0f, 32.0f, near_plane, far_plane)) {
            // Setup texture
            glGenTextures(1, &this->_depthmap);
            glBindTexture(GL_TEXTURE_2D, this->_depthmap);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->WIDTH, this->HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec4(1.0f)));

            // Setup framebuffer
            glGenFramebuffers(1, &this->_fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, this->_fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->_depthmap, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        inline ~ShadowMap() {
            glDeleteTextures(1, &this->_depthmap);
            glDeleteFramebuffers(1, &this->_fbo);
        }

        inline void update(const glm::vec3& eyePos, const glm::vec3& lDir, const glutils::PrimitiveRenderer& pr, UniformBufferObject& globals, const mcmodel::Drawable& scene) {
            glViewport(0, 0, this->WIDTH, this->HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, this->_fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            
            glutils::RenderContext ctx(pr, glutils::RenderContext::RenderPass::SHADOW_PASS, false);
            globals.setUniform("pass", (int) ctx.pass());
            globals.setUniform("lsm", this->_projection*glm::lookAt(eyePos - 3.0f*lDir, eyePos, glm::vec3(1.0f, 0.0f, 0.0f)));
            scene.draw(ctx);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        inline GLuint getTexture() const {
            return this->_depthmap;
        }
};

#endif