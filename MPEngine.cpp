#include "MPEngine.h"

#include <CSCI441/FixedCam.hpp>
#include <CSCI441/FreeCam.hpp>
#include <CSCI441/objects.hpp>

#include <cmath>
#include <string>

#include <glm/gtc/constants.hpp>

#include <iostream>

static constexpr GLfloat GLM_PI = glm::pi<float>();
static constexpr GLfloat GLM_2PI = glm::two_pi<float>();

/// Get GL error message
inline char const* get_gl_error_message(GLenum const err) noexcept {
    switch (err) {
        case GL_NO_ERROR: return "OK";
        case GL_INVALID_ENUM: return "invalid enum";
        case GL_INVALID_VALUE: return "invalid value";
        case GL_INVALID_OPERATION: return "invalid operation";
        case GL_STACK_OVERFLOW: return "stack overflow";
        case GL_STACK_UNDERFLOW: return "stack underflow";
        case GL_OUT_OF_MEMORY: return "out of memory";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "invalid framebuffer operation";
        default: return "unknown";
    }
}

/*** Engine Interface ***/

MPEngine::MPEngine()
    : CSCI441::OpenGLEngine(4, 1, 720, 720, "MP: Moria"),
    _freecam(nullptr),
    _primaryCamera(1),
    _secondaryCamera(1),
    _lastTime(0.0f),
    // Most objects have to be initialized later on once OpenGL is ready
    _shaderProgram(nullptr),
    _skyboxShaderProgram(nullptr),
    _im(nullptr),
    _tm(nullptr),
    _grid(nullptr),
    _skybox(nullptr),
    _block_planks(nullptr),
    _block_log(nullptr),
    _block_leaves(nullptr),
    _block_mushroom(nullptr),
    _block_amethyst(nullptr),
    _block_torch(nullptr),
    _block_red_spotlight(nullptr),
    _block_green_spotlight(nullptr),
    _block_blue_spotlight(nullptr),
    _player(nullptr),
    _player1(nullptr),
    _player2(nullptr),
    _player3(nullptr),
    _world()
{
    this->_tm = std::make_unique<glutils::TextureManager>();
    this->_im = std::make_unique<input::InputManager>();

    // Initialize random
    glutils::srandn(80801);

    // Initialize Input Bindings
    // ESC to exit
    this->_im->on({input::key(GLFW_KEY_ESCAPE)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        if(this->isDebuggingEnabled()) {
            fprintf(stdout, "[INFO]: User hit escape");
        }
        this->setWindowShouldClose();
    });

    // Left CTRL+C to exit
    this->_im->on({input::key(GLFW_KEY_C)}, {input::key(GLFW_KEY_LEFT_CONTROL)}, [this](GLFWwindow *const window, const float deltaTime) {
        if(this->isDebuggingEnabled()) {
            fprintf(stdout, "[INFO]: User hit control-c on window");
        }
        this->setWindowShouldClose();
    });

    // C to cycle between primary cameras (arcball, freecam) for debugging
    this->_im->on({input::key(GLFW_KEY_C)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->_primaryCamera = (this->_primaryCamera + 1) % 2;
    });

    // V to print position for debugging
    this->_im->on({input::key(GLFW_KEY_V)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        fprintf(stdout, "Player is at (%f, %f, %f)\n", this->_player->getPosition().x, this->_player->getPosition().y, this->_player->getPosition().z);
    });

    // Scroll changes arcball camera radius
    this->_im->on_axis(input::AxisType::Scroll, [this](const glm::vec2 offset) {
        if(this->_primaryCamera == 1) {
            this->_player->getArcballCamera().moveBackward(offset.y/2.5f);
        }
    });

    // W to move forward
    this->_im->on({input::key(GLFW_KEY_W)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        if(this->_primaryCamera == 0) {
            this->_freecam->moveForward(20.0f*deltaTime);
        } else {
            this->_player->setPosition(this->_player->getPosition() + this->_player->getHorizontalForwardVector()*5.0f*deltaTime);
        }
    }, input::Event::Hold);

    // S to move backward
    this->_im->on({input::key(GLFW_KEY_S)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        if(this->_primaryCamera == 0) {
            this->_freecam->moveBackward(20.0f*deltaTime);
        } else {
            this->_player->setPosition(this->_player->getPosition() - this->_player->getHorizontalForwardVector()*5.0f*deltaTime);
        }
    }, input::Event::Hold);

    // A to rotate right
    this->_im->on({input::key(GLFW_KEY_A)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->_player->setRotation(this->_player->getRotation() + glm::vec3(5.0f, 0.0f, 0.0f)*deltaTime);
    }, input::Event::Hold);

    // D to rotate left
    this->_im->on({input::key(GLFW_KEY_D)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->_player->setRotation(this->_player->getRotation() - glm::vec3(5.0f, 0.0f, 0.0f)*deltaTime);
    }, input::Event::Hold);

    // Cursor to rotate camera or zoom
    this->_im->on_axis(input::AxisType::Cursor, [this](const glm::vec2 pos) {
        if(this->_im->is_down(input::mouse(GLFW_MOUSE_BUTTON_LEFT)) && (this->_im->is_down(input::key(GLFW_KEY_LEFT_SHIFT)) || this->_im->is_down(input::key(GLFW_KEY_RIGHT_SHIFT)))) {
            if(this->_primaryCamera == 1) {
                this->_player->getArcballCamera().moveBackward((pos.y - this->_im->cursor().y)/12.0f);
            }
        } else if(this->_im->is_down(input::mouse(GLFW_MOUSE_BUTTON_LEFT))) {
            const GLfloat dTheta = 0.005*(pos.x - this->_im->cursor().x);  // Update yaw
            const GLfloat dPhi   = 0.005*(pos.y - this->_im->cursor().y);  // Update pitch
            const glm::vec2 scale = this->getPrimaryCameraRotationScale();

            // rotate the camera by the distance the mouse moved
            this->getPrimaryCamera()->rotate(dTheta*scale.x, dPhi*scale.y);
        }
    });

    // 1 to hide secondary camera
    this->_im->on({input::key(GLFW_KEY_1)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->_secondaryCamera = 0;
    });

    // 2 for first person camera
    this->_im->on({input::key(GLFW_KEY_2)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->_secondaryCamera = 1;
    });

    // 3 for sky camera
    this->_im->on({input::key(GLFW_KEY_3)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->_secondaryCamera = 2;
    });
}

MPEngine::~MPEngine() {}

/*** Engine Setup ***/

void MPEngine::mSetupGLFW() {
    CSCI441::OpenGLEngine::mSetupGLFW();

    // Set callbacks
    glfwSetKeyCallback(mpWindow, keyboard_callback);
    glfwSetMouseButtonCallback(mpWindow, mouse_button_callback);
    glfwSetCursorPosCallback(mpWindow, cursor_callback);
    glfwSetScrollCallback(mpWindow, scroll_callback);
}

void MPEngine::mSetupOpenGL() {
    glEnable(GL_DEPTH_TEST);					                    // Enable depth testing
    glDepthFunc(GL_LESS);							                // Use less than depth test

    glEnable(GL_BLEND);									            // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	            // Use one minus blending equation

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);	// Clear the frame buffer to gray
}

// Terrain shader and basic texture shader use some of the same uniforms
inline void initCommonFragmentShaderUniforms(const ShaderProgram& shader) {
    // Common uniforms
    shader.setProgramUniform("diffuseTexture", 0);
    shader.setProgramUniform("specularTexture", 1);
    shader.setProgramUniform("lit", true);

    // Positional Light (Torch)
    shader.setProgramUniform("torchPos", glm::vec3(20.5, 4.5, 20.5));
    shader.setProgramUniform("torchColor", glm::vec3(1.0f, 0.96f, 0.90f) /* Slight gold */);

    // Directional Light (Sun)
    shader.setProgramUniform("sunColor", glm::vec3(0.95, 1.0, 1.0) /* Slight blue */);
    shader.setProgramUniform("sunDirection", glm::vec3(1.0f, -1.0f, 1.0f));
    shader.setProgramUniform("sunIntensity", 0.85f);

    // Spot lights colors are hardcoded in shader
    shader.setProgramUniform("redSpotlightPos", glm::vec3(32.5f, 5.5f, 32.5f));
    shader.setProgramUniform("greenSpotlightPos", glm::vec3(30.5, 5.5, 32.5));
    shader.setProgramUniform("blueSpotlightPos", glm::vec3(31.5, 5.5, 32.5 + glm::sqrt(2)));
}

void MPEngine::mSetupShaders() {
    // Load shaders
    this->_shaderProgram = std::make_unique<ShaderProgram>("shaders/texshader.v.glsl", "shaders/texshader.f.glsl");
    initCommonFragmentShaderUniforms(*this->_shaderProgram);
    
    this->_skyboxShaderProgram = std::make_unique<ShaderProgram>("shaders/skybox.v.glsl", "shaders/skybox.f.glsl");
    this->_skyboxShaderProgram->setProgramUniform("skybox", 0);

    this->_terrainShaderProgram = std::make_unique<ShaderProgram>("shaders/terrain.v.glsl", "shaders/terrain.tc.glsl", "shaders/terrain.te.glsl", "shaders/texshader.f.glsl");
    initCommonFragmentShaderUniforms(*this->_terrainShaderProgram);
}

void MPEngine::mSetupBuffers() {
    // Make default.png texture handle 1, any textures that fail to load will fallback to this
    this->_tm->load("assets/textures/default.png");

    // Create skybox
    this->_skybox = std::make_shared<Skybox>(*this->_skyboxShaderProgram, std::array<std::string, 6> {
        "assets/textures/skybox/posx.jpg",
        "assets/textures/skybox/negx.jpg",
        "assets/textures/skybox/posy.jpg",
        "assets/textures/skybox/negy.jpg",
        "assets/textures/skybox/posz.jpg",
        "assets/textures/skybox/negz.jpg"
    });

    // Place ground grid
    this->_grid = mcmodel::TexturedFace::from(
        *this->_shaderProgram,
        {this->_tm->load("assets/textures/grid.png"), this->_tm->load("assets/textures/dull.png")},
        {
            glm::vec3 {0.0f, 0.0f, 0.0f},
            glm::vec3 {0.0f, 0.0f, World::WORLD_SIZE},
            glm::vec3 {World::WORLD_SIZE, 0.0f, World::WORLD_SIZE},
            glm::vec3 {World::WORLD_SIZE, 0.0f, 0.0f}
        }, {0.0f, 0.0f}, {World::WORLD_SIZE, World::WORLD_SIZE}
    );

    // Place terrain
    this->_terrain = TerrainPatch::from(*this->_terrainShaderProgram, World::WORLD_SIZE, {this->_tm->load("assets/textures/grass.png"), this->_tm->load("assets/textures/dull.png")});

    // Register blocks
    this->_block_planks = Block::from(mcmodel::cube(*this->_shaderProgram, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/planks.png"), this->_tm->load("assets/textures/dull.png")}}));
    this->_block_log = Block::from(mcmodel::cube(*this->_shaderProgram, std::array<std::array<GLuint, 2>, 6> {
        std::array<GLuint, 2> {this->_tm->load("assets/textures/log_side.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/log_side.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/log_top.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/log_top.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/log_side.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/log_side.png"), this->_tm->load("assets/textures/dull.png")}
    }));
    this->_block_leaves = Block::from(mcmodel::oscillate(*this->_shaderProgram, mcmodel::cube(*this->_shaderProgram, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/leaves.png"), this->_tm->load("assets/textures/shiny.png")}}), 0.5f), false);
    this->_block_amethyst = Block::from(mcmodel::cube(*this->_shaderProgram, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/amethyst.png"), this->_tm->load("assets/textures/shiny.png")}}));
    this->_block_mushroom = Block::from(mcmodel::cross(*this->_shaderProgram, {this->_tm->load("assets/textures/mushroom.png"), this->_tm->load("assets/textures/shiny.png")}), false);

    this->_block_torch = Block::from(mcmodel::ignore_light(*this->_shaderProgram, mcmodel::group({mcmodel::wrapped_cube(*this->_shaderProgram, std::array<GLuint, 2> {this->_tm->load("assets/textures/torch.png"), this->_tm->load("assets/textures/dull.png")}, {0.125f, 0.5f, 0.125f})}, {0.5f, 0.25f, 0.5f})), false);
    this->_block_red_spotlight = Block::from(mcmodel::cross(*this->_shaderProgram, {this->_tm->load("assets/textures/red_spotlight.png"), this->_tm->load("assets/textures/dull.png")}), false);
    this->_block_green_spotlight = Block::from(mcmodel::cross(*this->_shaderProgram, {this->_tm->load("assets/textures/green_spotlight.png"), this->_tm->load("assets/textures/dull.png")}), false);
    this->_block_blue_spotlight = Block::from(mcmodel::cross(*this->_shaderProgram, {this->_tm->load("assets/textures/blue_spotlight.png"), this->_tm->load("assets/textures/dull.png")}), false);

    this->_world = std::make_shared<World>();

    // Place one of each block for testing
    this->_world->setBlock(glm::ivec3(1, 0, 1), this->_block_planks);
    this->_world->setBlock(glm::ivec3(1, 0, 3), this->_block_log);
    this->_world->setBlock(glm::ivec3(1, 0, 5), this->_block_leaves);
    this->_world->setBlock(glm::ivec3(1, 0, 7), this->_block_amethyst);
    this->_world->setBlock(glm::ivec3(1, 0, 9), this->_block_mushroom);

    this->_world->setBlock(glm::ivec3(20, 4, 20), this->_block_torch);
    this->_world->setBlock(glm::ivec3(32, 5, 32), this->_block_red_spotlight);
    this->_world->setBlock(glm::ivec3(30, 5, 32), this->_block_green_spotlight);
    this->_world->setBlock(glm::ivec3(31, 5, 34), this->_block_blue_spotlight);

    // Place some trees
    this->_place_tree(this->_terrain->getTerrainPosition(10, 10));
    this->_place_tree(this->_terrain->getTerrainPosition(12, 38), 7);
    this->_place_tree(this->_terrain->getTerrainPosition(40, 30));
    this->_place_tree(this->_terrain->getTerrainPosition(32, 50), 5);
    this->_place_tree(this->_terrain->getTerrainPosition(10, 25), 8);
    this->_place_tree(this->_terrain->getTerrainPosition(50, 8));
    this->_place_tree(this->_terrain->getTerrainPosition(9, 56), 6);
    this->_place_tree(this->_terrain->getTerrainPosition(46, 50));

    // Place amethyst pyramid
    for(int dy = 0; dy < 3; dy++) {
        int r = 3 - dy - 1;
        for(int dx = -r; dx <= r; dx++) {
            for(int dz = -r; dz <= r; dz++) {
                this->_world->setBlock(glm::ivec3(20, this->_terrain->getTerrainHeight(20.5, 20.5), 20) + glm::ivec3(dx, dy, dz), this->_block_amethyst);
            }
        }
    }

    // Place collision test
    for(int dz = 0; dz < 9; dz++) {
        if(dz < 3 || dz > 5) {
            this->_world->setBlock(this->_terrain->getTerrainPosition(34.0f, 5.0f) + glm::vec3(0.0f, 0.0f, dz), this->_block_planks);
        }
        this->_world->setBlock(this->_terrain->getTerrainPosition(34.0f, 5.0f) + glm::vec3(0.0f, 1.0f, dz), this->_block_planks);
    }

    // Scatter mushrooms
    for(size_t i = 0; i < 100; i++) {
        const glm::ivec3 pos = this->_terrain->getTerrainPosition(glutils::randi(0, World::WORLD_SIZE), glutils::randi(0, World::WORLD_SIZE));
        // Check if ground mostly flat and empty
        if(this->_terrain->getTerrainHeight(pos.x, pos.z) - pos.y < 0.125 && !this->_world->getBlock(pos)) {
            this->_world->setBlock(pos, this->_block_mushroom);
        }
    }

    // Setup players
    this->_player1 = std::make_shared<Player>(this->_world, *this->_shaderProgram, std::array<GLuint, 2> {this->_tm->load("assets/textures/idril.png"), this->_tm->load("assets/textures/idril_specular.png")}, true, std::array<GLuint, 2> {this->_tm->load("assets/textures/cape.png"), this->_tm->load("assets/textures/cape_specular.png")});
    this->_player1->setPosition(this->_terrain->getTerrainPosition(32.0f, 32.0f));
    this->_player = this->_player1.get();

    this->_player2 = std::make_shared<Player>(this->_world, *this->_shaderProgram, std::array<GLuint, 2> {this->_tm->load("assets/textures/idril.png"), this->_tm->load("assets/textures/idril_specular.png")}, true, std::array<GLuint, 2> {this->_tm->load("assets/textures/cape2.png"), this->_tm->load("assets/textures/cape2_specular.png")});
    this->_player2->setPosition(this->_terrain->getTerrainPosition(35.0f, 32.0f));

    this->_player3 = std::make_shared<Player>(this->_world, *this->_shaderProgram, std::array<GLuint, 2> {this->_tm->load("assets/textures/idril.png"), this->_tm->load("assets/textures/idril_specular.png")}, true);
    this->_player3->setPosition(this->_terrain->getTerrainPosition(35.0f, 35.0f));
}

/// Generates a tree at pos with variable log height
void MPEngine::_place_tree(const glm::ivec3 pos, const size_t height) {
    for(int dy = 0; dy <= height; dy++) {
        this->_world->setBlock(pos + glm::ivec3(0, dy, 0), dy == height ? this->_block_leaves : this->_block_log);
        
        if(dy > height - 2) {
            for(int dx = -1; dx <= 1; dx++) {
                for(int dz = -1; dz <= 1; dz++) {
                    if((dx == 0) ^ (dz == 0)) {
                        this->_world->setBlock(pos + glm::ivec3(dx, dy, dz), this->_block_leaves);
                    }
                }
            }
        } else if(dy > height - 4) {
            for(int dx = -2; dx <= 2; dx++) {
                for(int dz = -2; dz <= 2; dz++) {
                    if((dx != 0) || (dz != 0)) {
                        this->_world->setBlock(pos + glm::ivec3(dx, dy, dz), this->_block_leaves);
                    }
                }
            }
        }
    }
}

void MPEngine::mSetupScene() {
    this->_freecam = std::make_shared<CSCI441::FreeCam>((GLfloat)mWindowWidth / (GLfloat)mWindowHeight, 45.0f, 0.001f, 1000.0f);
    this->_freecam->setPosition(glm::vec3(-4.0f, 4.0f, -4.0f));
    this->_freecam->setTheta(-1.25f*M_PI);
    this->_freecam->setPhi(M_PI / 2.8f);
    this->_freecam->recomputeOrientation();

    // All other cameras are handled by the Player

    _lastTime = (GLfloat)glfwGetTime();
}

/*** Engine Cleanup ***/
void MPEngine::mCleanupShaders() {
    fprintf( stdout, "[INFO]: ...deleting Shaders.\n" );
    this->_shaderProgram = nullptr;
    this->_skyboxShaderProgram = nullptr;
    this->_terrainShaderProgram = nullptr;
}

void MPEngine::mCleanupBuffers() {
    fprintf( stdout, "[INFO]: ...deleting VAOs....\n" );
    this->_world = nullptr;
    this->_block_planks = nullptr;
    this->_block_log = nullptr;
    this->_block_leaves = nullptr;
    this->_block_mushroom = nullptr;
    this->_block_amethyst = nullptr;
    this->_block_torch = nullptr;
    this->_block_red_spotlight = nullptr;
    this->_block_green_spotlight = nullptr;
    this->_block_blue_spotlight = nullptr;
    this->_grid = nullptr;
    this->_skybox = nullptr;
    this->_player = nullptr;
    this->_player1 = nullptr;
    this->_player2 = nullptr;
    this->_player3 = nullptr;
    this->_terrain = nullptr;
}

void MPEngine::mCleanupTextures() {
    fprintf( stdout, "[INFO]: ...deleting textures\n" );
    this->_tm = nullptr;
}

void MPEngine::mCleanupScene() {
    fprintf( stdout, "[INFO]: ...deleting scene...\n" );
    this->_freecam = nullptr;
}

/*** Camera Utility ***/

CSCI441::Camera* MPEngine::getPrimaryCamera() const {
    switch(_primaryCamera) {
        case 1:
            return &this->_player->getArcballCamera();
        default:
            return this->_freecam.get();
    }
}

glm::vec2 MPEngine::getPrimaryCameraRotationScale() const {
    switch(_primaryCamera) {
        case 1:
            return glm::vec2(1.0f, -1.0f);
        default:
            return glm::vec2(1.0f, 1.0f);
    }
}

CSCI441::Camera* MPEngine::getSecondaryCamera() const {
    switch(_secondaryCamera) {
        case 1:
            return &this->_player->getFirstPersonCamera();
        case 2:
            return &this->_player->getSkyCamera();
        default:
            return nullptr;
    }
}

/*** Rendering/Drawing Functions ***/

void drawTerrainAlignedPlayer(glutils::RenderContext& ctx, const Player& player, const TerrainPatch& terrain) {
    ctx.pushTransformation(glm::translate(glm::mat4(1.0f), player.getPosition())*glm::toMat4(glm::rotation(glm::vec3(0.0, -1.0, 0.0), terrain.getTerrainNormal(player.getPosition().x, player.getPosition().z)))*glm::translate(glm::mat4(1.0f), -player.getPosition()));
    player.draw(ctx);
    ctx.popTransformation();
}

void MPEngine::_renderScene(glutils::RenderContext& ctx) const {
    this->_skybox->draw(ctx);

    this->_terrain->draw(ctx);

    this->_shaderProgram->useProgram();
    ctx.bind(*this->_shaderProgram);

    this->_grid->draw(ctx);
    this->_world->draw(ctx);

    this->_player1->draw(ctx);
    // drawTerrainAlignedPlayer(ctx, *this->_player1, *this->_terrain);
    this->_player2->draw(ctx);
    this->_player3->draw(ctx);
}

void MPEngine::_updateScene() {
    GLfloat currTime = (GLfloat)glfwGetTime();
    GLfloat deltaTime = currTime - _lastTime;
    this->_lastTime = currTime;

    this->_shaderProgram->setProgramUniform("time", currTime);

    // Update keybinds
    this->_im->poll(this->mpWindow, deltaTime);

    // Update players
    this->_player1->update(deltaTime);
    this->_player2->update(deltaTime);
    this->_player3->update(deltaTime);

    // Move with terrain
    this->_player->setPosition(this->_terrain->getTerrainPosition(this->_player->getPosition().x, this->_player->getPosition().z));

    // This isn't quite right, but it looks fine... so it's good enough
    const glm::vec3 terrainRotation = glm::eulerAngles(glm::rotation(glm::vec3(0.0, -1.0, 0.0), this->_terrain->getTerrainNormal(this->_player->getPosition().x, this->_player->getPosition().z)));
    this->_player->setRotation({this->_player->getRotation().x , terrainRotation.y*cos(this->_player->getRotation().x), terrainRotation.z*cos(this->_player->getRotation().x)});
}

void MPEngine::run() {
    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while(!glfwWindowShouldClose(mpWindow)) {	            // check if the window was instructed to be closed
        glDrawBuffer(GL_BACK);		                        // work with our back frame buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// clear the current color contents and depth buffer in the window

        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore,
        // query what the actual size of the window we are rendering to is.
        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(mpWindow, &framebufferWidth, &framebufferHeight);

        // Update the scene
        _updateScene();

        // Draw primary camera to the whole window
        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glutils::RenderContext ctx(*this->getPrimaryCamera());
        _renderScene(ctx);

        // Optional secondary cameras
        if(this->getSecondaryCamera() != nullptr) {
            
            // Clear just the region of the mini-viewport
            glEnable(GL_SCISSOR_TEST);
            glScissor(framebufferWidth*0.75, framebufferHeight*0.75, framebufferWidth*0.25, framebufferHeight*0.25);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);
            
            // Draw secondary camera
            glViewport(framebufferWidth*0.75, framebufferHeight*0.75, framebufferWidth*0.25, framebufferHeight*0.25);
            glutils::RenderContext ctx(*this->getSecondaryCamera());
            
            if(this->getSecondaryCamera() == &this->_player->getFirstPersonCamera()) {
                this->_player->setHidden(true); // Hide in first person view
            }
            _renderScene(ctx);

            // Reset
            glViewport(0, 0, framebufferWidth, framebufferHeight);
            this->_player->setHidden(false);
        }
        
        glfwSwapBuffers(mpWindow); // Flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents(); // Check for any events and signal to redraw screen
    }
}

// Need to expose to callbacks
input::InputManager& MPEngine::getInputManager() {
    return *this->_im;
}

/*** Callbacks (forward to input manager) ***/
void keyboard_callback(GLFWwindow *window, const int key, const int scancode, const int action, const int mods) {
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));
    if(key != GLFW_KEY_UNKNOWN) {
        if(action == GLFW_PRESS) {
            engine->getInputManager().dispatch(window, input::key(key), input::State::Press);
        } else if(action == GLFW_RELEASE) {
            engine->getInputManager().dispatch(window, input::key(key), input::State::Release);
        }
    }
}

void cursor_callback(GLFWwindow *window, const double x, const double y) {
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));
    engine->getInputManager().cursor(glm::vec2(x, engine->getWindowHeight() - y));
}

void mouse_button_callback(GLFWwindow *window, const int button, const int action, const int mods) {
    auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));
    if(action == GLFW_PRESS) {
        engine->getInputManager().dispatch(window, input::mouse(button), input::State::Press);
    } else if(action == GLFW_RELEASE) {
        engine->getInputManager().dispatch(window, input::mouse(button), input::State::Release);
    }
}

void scroll_callback(GLFWwindow *window, const double xOffset, const double yOffset) {
    const auto engine = static_cast<MPEngine*>(glfwGetWindowUserPointer(window));
    engine->getInputManager().scroll(glm::vec2(xOffset, yOffset));
}
