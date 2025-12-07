#ifndef MCEngine_H
#define MCEngine_H

/*
 * MCEngine.hpp
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
#include "PrimitiveRenderer.hpp"
#include "input.hpp"
#include "mcmodel.hpp"
#include "glutils.hpp"
#include "Player.hpp"
#include "Block.hpp"
#include "World.hpp"
#include "Skybox.hpp"
#include "Clouds.hpp"
#include "md5camera.hpp"
#include "UniformBufferObject.hpp"

class MCEngine final : public CSCI441::OpenGLEngine {
    public:
        /*** Engine Interface ***/
        MCEngine(const std::string& player_name = "Idril");
        ~MCEngine() override;

        void run() override;

        input::InputManager& getInputManager();

        void initialize() override;
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
        /// Draws HUD elements
        void _renderHUD(glutils::RenderContext& ctx) const;
        /// Handles animations and updating camera
        void _updateScene();
        
        /// Read debug commands from terminal
        void _handleConsoleInput();

        /// If debug info should be drawn, separate from debug logging
        bool _debug = false;

        /// Time last frame was rendered
        GLfloat _lastTime;

        /*** Camera Information ***/
        /// A free camera for debugging
        std::shared_ptr<CSCI441::FreeCam> _freecam;

        /// A camera for movies
        std::shared_ptr<CSCI441::FixedCam> _fixedcam;

        /// Index of current primary camera [Free Camera, Player Arcball Camera]
        unsigned int _primaryCamera;
        CSCI441::Camera* getPrimaryCamera() const;

        /// Not all cameras rotate the same, allow adjusting rate and inverting
        glm::vec2 getPrimaryCameraRotationScale() const;

        /// Index of secondary camera [None, First Person Camera, Sky Camera]
        unsigned int _secondaryCamera;
        CSCI441::Camera* getSecondaryCamera() const;

        /// Frame by frame camera config
        md5camera::MD5Movie _movie;

        /*** Shader Program Information ***/
        struct {
            /// Shader program that performs texturing and Phong shading
            std::unique_ptr<ShaderProgram> primary;
            /// Cubemap shader
            std::unique_ptr<ShaderProgram> skybox;
            /// Cloud shader
            std::unique_ptr<ShaderProgram> clouds;
            /// Terrain Patch Shader
            std::unique_ptr<ShaderProgram> terrain;

            /// Simple cube outline shader
            std::unique_ptr<ShaderProgram> cube;

            /// Simple line shader
            std::unique_ptr<ShaderProgram> line;

            /// Simple point shader
            std::unique_ptr<ShaderProgram> point;

            /// Simple ui rectangle shader with alpha
            std::unique_ptr<ShaderProgram> rect;

            /// Textured sprite shader
            std::unique_ptr<ShaderProgram> sprite;
        } _shaders;

        std::shared_ptr<UniformBufferObject> _shader_globals;

        /*** Helper Classes ***/
        std::unique_ptr<input::InputManager> _im;
        std::unique_ptr<glutils::TextureManager> _tm;
        std::unique_ptr<PrimitiveRenderer> _pr;

        /*** Objects ***/
        std::shared_ptr<Skybox> _skybox;
        std::shared_ptr<Clouds> _clouds;
        std::shared_ptr<mcmodel::Drawable> _grid;

        std::shared_ptr<World> _world;

        const std::string _player_name;
        std::shared_ptr<Player> _player;


        std::unordered_map<std::string, std::shared_ptr<Block>> _blocks;

        /// Generates a tree at pos with variable log height
        void _place_tree(const glm::ivec3 pos, const size_t height = 5);
};

/*** C Callbacks ***/

void keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void cursor_callback(GLFWwindow *window, double x, double y);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xOffset, double yOffset);

#endif // MCEngine_H
