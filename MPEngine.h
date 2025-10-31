#ifndef A3ENGINE_H
#define A3ENGINE_H

/*
 * A3Engine.hpp
 * Trin Wasinger - Fall 2025
 *
 * The core of this project, manages world, input, updating scene, rendering, most object lifetimes, etc...
 */

#include <CSCI441/FreeCam.hpp>
#include <CSCI441/MD5Model.hpp>
#include <CSCI441/ModelLoader.hpp>
#include <CSCI441/OpenGLEngine.hpp>

#include <memory>
#include <unordered_map>

#include "ShaderProgram.hpp"
#include "input.hpp"
#include "mcmodel.hpp"
#include "glutils.hpp"
#include "Player.hpp"
#include "Block.hpp"
#include "World.hpp"

class A3Engine final : public CSCI441::OpenGLEngine {
    public:
        /*** Engine Interface ***/
        A3Engine();
        ~A3Engine() override;

        void run() override;

        input::InputManager& getInputManager();
    private:
        /*** Engine Setup ***/
        void mSetupGLFW() override;
        void mSetupOpenGL() override;
        void mSetupShaders() override;
        void mSetupBuffers() override;
        // void mSetupTextures() override; handled by TextureManager
        void mSetupScene() override;

        /*** Engine Cleanup ***/
        void mCleanupScene() override;
        void mCleanupTextures() override;
        void mCleanupBuffers() override;
        void mCleanupShaders() override;

        /*** Engine Rendering & Updating ***/
        /// Draws everything to the scene from a particular point of view
        void _renderScene(glutils::RenderContext& ctx) const;
        /// Handles animations and updating camera
        void _updateScene();
        
        /*** Camera Information ***/
        /// A free camera for debugging
        std::shared_ptr<CSCI441::FreeCam> _freecam;

        /// Index of current primary camera [Free Camera, Player Arcball Camera]
        unsigned int _primaryCamera;
        CSCI441::Camera* getPrimaryCamera() const;

        /// Not all cameras rotate the same, allow adjusting rate and inverting
        glm::vec2 getPrimaryCameraRotationScale() const;

        /// Index of secondary camera [None, First Person Camera, Sky Camera]
        unsigned int _secondaryCamera;
        CSCI441::Camera* getSecondaryCamera() const;

        /// time last frame was rendered
        GLfloat _lastTime;

        /*** Shader Program Information ***/

        /// Shader program that performs texturing and Phong shading
        std::unique_ptr<ShaderProgram> _shaderProgram;

        /*** Helper Classes ***/
        std::unique_ptr<input::InputManager> _im;
        std::unique_ptr<glutils::TextureManager> _tm;

        /*** Objects ***/
        std::shared_ptr<mcmodel::Drawable> _skybox;
        std::shared_ptr<mcmodel::Drawable> _grid;
        std::shared_ptr<Block> _block_planks;
        std::shared_ptr<Block> _block_log;
        std::shared_ptr<Block> _block_leaves;
        std::shared_ptr<Block> _block_mushroom;
        std::shared_ptr<Block> _block_amethyst;
        std::shared_ptr<World> _world;
        std::shared_ptr<Player> _player;

        /// Generates a tree at pos with variable log height
        void _place_tree(const glm::ivec3 pos, const size_t height = 5);
};

/*** C Callbacks ***/

void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void cursor_callback(GLFWwindow *window, double x, double y);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xOffset, double yOffset);

#endif// A3ENGINE_H
