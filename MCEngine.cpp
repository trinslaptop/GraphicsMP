#include "MCEngine.h"

#include <CSCI441/FixedCam.hpp>
#include <CSCI441/FreeCam.hpp>
#include <CSCI441/objects.hpp>

#include <cmath>
#include <string>
#include <iostream>
#include <stdint.h>

#include <glm/gtc/constants.hpp>

#include "include/f8.hpp"

#include "get_player.hpp"
#include "Zombie.hpp"

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

MCEngine::MCEngine(const std::string& player_name)
    : CSCI441::OpenGLEngine(4, 1, 720, 720, "Minceraft"),
    _player_name(player_name),
    _freecam(nullptr),
    _fixedcam(nullptr),
    _primaryCamera(1),
    _secondaryCamera(1),
    _lastTime(0.0f),
    // Most objects have to be initialized later on once OpenGL is ready
    _im(nullptr),
    _tm(nullptr),
    _pr(nullptr),
    _grid(nullptr),
    _skybox(nullptr),
    _clouds(nullptr),
    _player(nullptr),
    _blocks(),
    _world()
{
    this->_tm = std::make_unique<glutils::TextureManager>();
    this->_im = std::make_unique<input::InputManager>();

    // Initialize Input Bindings
    // ESC to exit
    this->_im->on({input::key(GLFW_KEY_ESCAPE)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        if(this->isDebuggingEnabled()) {
            fprintf(stdout, "\n[INFO]: User hit escape\n");
        }
        this->setWindowShouldClose();
    });

    // Left CTRL+C to exit
    this->_im->on({input::key(GLFW_KEY_C)}, {input::key(GLFW_KEY_LEFT_CONTROL)}, [this](GLFWwindow *const window, const float deltaTime) {
        if(this->isDebuggingEnabled()) {
            fprintf(stdout, "\n[INFO]: User hit control-c on window\n");
        }
        this->setWindowShouldClose();
    });

    // F5 to switch between arcball and freecam in primary viewport
    // Note: can't manually select movie camera
    this->_im->on({input::key(GLFW_KEY_F5)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        if(!this->_movie.empty()) return;
        this->_primaryCamera = (this->_primaryCamera + 1) % 2;
    });

    // F4 to switch between first person, sky, and none in secondary viewport
    this->_im->on({input::key(GLFW_KEY_F4)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        if(!this->_movie.empty()) return;
        this->_secondaryCamera = (this->_secondaryCamera + 1) % 3;
    });

    // Z to print position for debugging
    this->_im->on({input::key(GLFW_KEY_Z)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        fprintf(stdout, "Player is at (%f, %f, %f)\n", this->_player->getPosition().x, this->_player->getPosition().y, this->_player->getPosition().z);
    });

    // Scroll changes arcball camera radius
    this->_im->on_axis(input::AxisType::Scroll, [this](const glm::vec2 offset) {
        if(this->_primaryCamera == 1) {
            this->_player->getArcballCamera().moveForward(offset.y/2.5f);
        }
    });

    // W to move forward
    this->_im->on({input::key(GLFW_KEY_W)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        if(this->_primaryCamera == 0) {
            this->_freecam->moveForward(20.0f*deltaTime);
        } else {
            this->_player->setPosition(this->_player->getPosition() + this->_player->getForwardVector()*5.0f*deltaTime);
        }
    }, input::Event::Hold);

    // S to move backward
    this->_im->on({input::key(GLFW_KEY_S)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        if(this->_primaryCamera == 0) {
            this->_freecam->moveBackward(20.0f*deltaTime);
        } else {
            this->_player->setPosition(this->_player->getPosition() - this->_player->getForwardVector()*5.0f*deltaTime);
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

    // SPACE to jump
    this->_im->on({input::key(GLFW_KEY_SPACE)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->_player->jump();
    });

    // Konami cheat code for invulnerability
    this->_im->on({input::key(GLFW_KEY_UP), input::key(GLFW_KEY_UP), input::key(GLFW_KEY_DOWN), input::key(GLFW_KEY_DOWN), input::key(GLFW_KEY_LEFT), input::key(GLFW_KEY_RIGHT), input::key(GLFW_KEY_LEFT), input::key(GLFW_KEY_RIGHT)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->_player->setInvulnerable(!this->_player->isInvulnerable());
    }, input::Event::Press, 1.0f);

    // Cursor to rotate camera or zoom
    this->_im->on_axis(input::AxisType::Cursor, [this](const glm::vec2 pos) {
        if(this->_im->is_down(input::mouse(GLFW_MOUSE_BUTTON_LEFT)) && (this->_im->is_down(input::key(GLFW_KEY_LEFT_SHIFT)) || this->_im->is_down(input::key(GLFW_KEY_RIGHT_SHIFT)))) {
            if(this->_primaryCamera == 1) {
                this->_player->getArcballCamera().moveForward((pos.y - this->_im->cursor().y)/12.0f);
            }
        } else if(this->_im->is_down(input::mouse(GLFW_MOUSE_BUTTON_LEFT))) {
            const GLfloat dTheta = 0.005*(pos.x - this->_im->cursor().x);  // Update yaw
            const GLfloat dPhi   = 0.005*(pos.y - this->_im->cursor().y);  // Update pitch
            const glm::vec2 scale = this->getPrimaryCameraRotationScale();

            // rotate the camera by the distance the mouse moved
            this->getPrimaryCamera()->rotate(dTheta*scale.x, dPhi*scale.y);
        }
    });

    // F2 for screenshot
    this->_im->on({input::key(GLFW_KEY_F2)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->saveScreenshot(NULL);
    });

    // F3 for debug info
    this->_im->on({input::key(GLFW_KEY_F3)}, {}, [this](GLFWwindow *const window, const float deltaTime) {
        this->_debug = !this->_debug;
    });
}

MCEngine::~MCEngine() {}

/*** Engine Setup ***/

void MCEngine::initialize() {
    CSCI441::OpenGLEngine::initialize();

    // Needed for readasync()
    std::ios_base::sync_with_stdio(false);
    fprintf(stdout, "> ");
    fflush(stdout);
}

void MCEngine::mSetupGLFW() {
    CSCI441::OpenGLEngine::mSetupGLFW();

    // Request higher accuracy depth buffer
    glfwWindowHint(GLFW_DEPTH_BITS, 32);

    // Anti-aliasing
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Set callbacks
    glfwSetKeyCallback(mpWindow, keyboard_callback);
    glfwSetMouseButtonCallback(mpWindow, mouse_button_callback);
    glfwSetCursorPosCallback(mpWindow, cursor_callback);
    glfwSetScrollCallback(mpWindow, scroll_callback);
}

void MCEngine::mSetupOpenGL() {
    glEnable(GL_DEPTH_TEST);					                    // Enable depth testing
    glDepthFunc(GL_LESS);							                // Use less than depth test

    glEnable(GL_BLEND);									            // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	            // Use one minus blending equation

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);	// Clear the frame buffer to gray

    // Anti-aliasing
    glEnable(GL_MULTISAMPLE);
}

// Terrain shader and basic texture shader use some of the same uniforms
inline void initCommonFragmentShaderUniforms(const ShaderProgram& shader) {
    // Common uniforms
    shader.setProgramUniform("diffuseTexture", 0);
    shader.setProgramUniform("specularTexture", 1);
    shader.setProgramUniform("lit", true);
    shader.setProgramUniform("frameCount", (GLuint) 1);
    shader.setProgramUniform("frameTime", 1.0f);
    shader.setProgramUniform("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Positional Light (Torch)
    shader.setProgramUniform("torchPos", glm::vec3(17.5, 5.5, 17.5));
    shader.setProgramUniform("torchColor", glm::vec3(1.0f, 0.96f, 0.90f) /* Slight gold */);

    // Directional Light (Sun)
    shader.setProgramUniform("sunColor", glm::vec3(0.95, 1.0, 1.0) /* Slight blue */);
    shader.setProgramUniform("sunDirection", glm::vec3(1.0f, -1.0f, 1.0f));
    shader.setProgramUniform("sunIntensity", 0.85f);
}

void MCEngine::mSetupShaders() {
    this->_shader_globals = std::make_shared<UniformBufferObject>(0);

    // Load shaders
    this->_shaders.primary = std::make_unique<ShaderProgram>("shaders/texshader.v.glsl", "shaders/texshader.f.glsl");
    initCommonFragmentShaderUniforms(*this->_shaders.primary);
    this->_shader_globals->bindShaderBlock(*this->_shaders.primary, "Globals");

    this->_shaders.skybox = std::make_unique<ShaderProgram>("shaders/skybox/skybox.v.glsl", "shaders/skybox/skybox.f.glsl");
    this->_shader_globals->bindShaderBlock(*this->_shaders.skybox, "Globals");
    this->_shaders.skybox->setProgramUniform("skybox", 0);

    this->_shaders.clouds = std::make_unique<ShaderProgram>("shaders/clouds/clouds.v.glsl", "shaders/clouds/clouds.f.glsl");
    this->_shader_globals->bindShaderBlock(*this->_shaders.clouds, "Globals");
    this->_shaders.clouds->setProgramUniform("clouds", 0);

    this->_shaders.cube = std::make_unique<ShaderProgram>("shaders/nop.glsl", "shaders/primitives/cube.g.glsl", "shaders/solidcolor.f.glsl");
    this->_shader_globals->bindShaderBlock(*this->_shaders.cube, "Globals");
    
    this->_shaders.line = std::make_unique<ShaderProgram>("shaders/nop.glsl", "shaders/primitives/line.g.glsl", "shaders/solidcolor.f.glsl");
    this->_shader_globals->bindShaderBlock(*this->_shaders.line, "Globals");

    this->_shaders.point = std::make_unique<ShaderProgram>("shaders/primitives/point.v.glsl", "shaders/solidcolor.f.glsl");
    this->_shader_globals->bindShaderBlock(*this->_shaders.point, "Globals");

    this->_shaders.rect = std::make_unique<ShaderProgram>("shaders/nop.glsl", "shaders/primitives/rect.g.glsl", "shaders/alphacolor.f.glsl");

    this->_shaders.sprite = std::make_unique<ShaderProgram>("shaders/nop.glsl", "shaders/primitives/sprite.g.glsl", "shaders/texshader.f.glsl");
    this->_shader_globals->bindShaderBlock(*this->_shaders.sprite, "Globals");
    initCommonFragmentShaderUniforms(*this->_shaders.primary);

    this->_shaders.terrain = std::make_unique<ShaderProgram>("shaders/terrain/terrain.v.glsl", "shaders/terrain/terrain.tc.glsl", "shaders/terrain/terrain.te.glsl", "shaders/texshader.f.glsl");
    initCommonFragmentShaderUniforms(*this->_shaders.terrain);
    this->_shader_globals->bindShaderBlock(*this->_shaders.terrain, "Globals");

}

void MCEngine::mSetupBuffers() {
    // Initialize random number generators (specific state and global ones)
    const unsigned int seed = 0x80801;
    uint32_t state;
    f8::srand(seed, state);
    f8::srand();
    f8::srandv();

    // Make default.png texture handle 1, any textures that fail to load will fallback to this
    this->_tm->load("assets/textures/default.png");
    this->_tm->load("assets/textures/dull.png");
    this->_tm->load("assets/textures/shiny.png");

    this->_pr = std::make_unique<glutils::PrimitiveRenderer>(*this->_shaders.cube, *this->_shaders.line, *this->_shaders.point, *this->_shaders.rect, *this->_shaders.sprite, this->_tm->load("assets/textures/sprites.png"));

    // Create skybox
    this->_skybox = std::make_shared<Skybox>(*this->_shaders.skybox, std::array<std::string, 6> {
        "assets/textures/skybox/posx.jpg",
        "assets/textures/skybox/negx.jpg",
        "assets/textures/skybox/posy.jpg",
        "assets/textures/skybox/negy.jpg",
        "assets/textures/skybox/posz.jpg",
        "assets/textures/skybox/negz.jpg"
    });

    this->_clouds = Clouds::from(*this->_shaders.clouds, this->_tm->load("assets/textures/clouds.png"));

    // Place ground grid
    this->_grid = mcmodel::TexturedFace::from(
        *this->_shaders.primary,
        {this->_tm->load("assets/textures/grid.png"), this->_tm->load("assets/textures/dull.png")},
        {
            glm::vec3 {0.0f, 0.0f, 0.0f},
            glm::vec3 {0.0f, 0.0f, 4*Chunk::CHUNK_SIZE},
            glm::vec3 {4*Chunk::CHUNK_SIZE, 0.0f, 4*Chunk::CHUNK_SIZE},
            glm::vec3 {4*Chunk::CHUNK_SIZE, 0.0f, 0.0f}
        }, {0.0f, 0.0f}, {4*Chunk::CHUNK_SIZE, 4*Chunk::CHUNK_SIZE}
    );

    // Register blocks
    this->_blocks["air"] = nullptr;
    this->_blocks["planks"] = Block::from(mcmodel::cube(*this->_shaders.primary, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/block/planks.png"), this->_tm->load("assets/textures/dull.png")}}));
    this->_blocks["glass"] = Block::from(mcmodel::cube(*this->_shaders.primary, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/block/glass.png"), this->_tm->load("assets/textures/shiny.png")}}));
    this->_blocks["log"] = Block::from(mcmodel::cube(*this->_shaders.primary, std::array<std::array<GLuint, 2>, 6> {
        std::array<GLuint, 2> {this->_tm->load("assets/textures/block/log_side.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/block/log_side.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/block/log_top.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/block/log_top.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/block/log_side.png"), this->_tm->load("assets/textures/dull.png")},
        std::array<GLuint, 2> {this->_tm->load("assets/textures/block/log_side.png"), this->_tm->load("assets/textures/dull.png")}
    }));
    this->_blocks["leaves"] = Block::from(mcmodel::oscillate(*this->_shaders.primary, mcmodel::cube(*this->_shaders.primary, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/block/leaves.png"), this->_tm->load("assets/textures/shiny.png")}}), 0.5f), false);
    this->_blocks["amethyst"] = Block::from(mcmodel::cube(*this->_shaders.primary, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/block/amethyst.png"), this->_tm->load("assets/textures/shiny.png")}}));
    this->_blocks["mushroom"] = Block::from(
        mcmodel::animtex(*this->_shaders.primary, mcmodel::cross(*this->_shaders.primary, {this->_tm->load("assets/textures/block/mushroom_anim.png"), this->_tm->load("assets/textures/shiny.png")}), 4, 8.0f),
        false
    );
    this->_blocks["tall_grass"] = Block::from(
        mcmodel::group({mcmodel::tint(*this->_shaders.primary, mcmodel::oscillate(*this->_shaders.primary, mcmodel::cross(*this->_shaders.primary, {this->_tm->load("assets/textures/block/tall_grass.png"), this->_tm->load("assets/textures/dull.png")})), glm::vec4(0.19f, 0.5f, 0.0f, 1.0f))}, glm::vec3(0.0f, -0.0625f, 0.0f)),
        false
    );
    this->_blocks["torch"] = Block::from(mcmodel::ignore_light(*this->_shaders.primary, mcmodel::group({mcmodel::wrapped_cube(*this->_shaders.primary, std::array<GLuint, 2> {this->_tm->load("assets/textures/block/torch.png"), this->_tm->load("assets/textures/dull.png")}, {0.125f, 0.5f, 0.125f})}, {0.5f, 0.25f, 0.5f})), false);

    // We do what we must because we can
    this->_blocks["cube"] = Block::from(
        mcmodel::group({
            mcmodel::cube(*this->_shaders.primary, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/block/cube/base.png"), this->_tm->load("assets/textures/block/cube/base_shiny.png")}}),
            mcmodel::ignore_light(*this->_shaders.primary, mcmodel::tint(*this->_shaders.primary, mcmodel::cube(*this->_shaders.primary, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/block/cube/lines.png"), this->_tm->load("assets/textures/dull.png")}}), glm::vec3(1.0f, 0.28f, 1.0f))),
            mcmodel::group({
                mcmodel::cube(*this->_shaders.primary, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/block/cube/shell.png"), this->_tm->load("assets/textures/block/cube/shell_shiny.png")}}),
                mcmodel::ignore_light(*this->_shaders.primary, mcmodel::tint(*this->_shaders.primary, mcmodel::cube(*this->_shaders.primary, std::array<std::array<GLuint, 2>, 1> {{this->_tm->load("assets/textures/block/cube/heart.png"), this->_tm->load("assets/textures/dull.png")}}), glm::vec3(1.0f, 0.28f, 1.0f))),
            }, glm::vec3(-0.03125f), glm::vec3(0.0f), glm::vec3(1.0625f))
        })
    );

    this->_world = std::make_shared<World>(seed, *this->_shaders.primary, *this->_shaders.terrain, std::array<GLuint, 2> {this->_tm->load("assets/textures/block/grass.png"), this->_tm->load("assets/textures/dull.png")});

    // Initialize chunks
    for(size_t ckx = 0; ckx < 4; ckx++) {
        for(size_t ckz = 0; ckz < 4; ckz++) {
            this->_world->getChunk({ckx, 0, ckz});
        }
    }

    this->_world->setBlock(glm::ivec3(63, 0, 63), this->_blocks["cube"]);

    // Place some trees
    this->_place_tree({10, this->_world->getTerrainHeight(10.5f, 10.5f) - 0.5f, 10});
    this->_place_tree({12, this->_world->getTerrainHeight(12.5f, 38.5f) - 1.0f, 12}, 7);
    this->_place_tree({40, this->_world->getTerrainHeight(40.5f, 30.5f) - 0.5f, 40});
    this->_place_tree({32, this->_world->getTerrainHeight(32.5f, 50.5f) - 0.5f, 50}, 5);
    this->_place_tree({10, this->_world->getTerrainHeight(10.5f, 25.5f) - 0.5f, 25}, 8);
    this->_place_tree({50, this->_world->getTerrainHeight(50.5f, 8.5f) - 0.5f, 8});
    this->_place_tree({9, this->_world->getTerrainHeight(9.5f, 56.5f) - 0.5f, 56}, 6);
    this->_place_tree({46, this->_world->getTerrainHeight(46.5f, 50.5f) - 0.5f, 50});

    // Place amethyst pyramid
    this->_world->setBlock(glm::ivec3(17, this->_world->getTerrainHeight(17.5, 17.5) + 3, 17), this->_blocks["torch"]);
    for(int dy = 0; dy < 3; dy++) {
        int r = 3 - dy - 1;
        for(int dx = -r; dx <= r; dx++) {
            for(int dz = -r; dz <= r; dz++) {
                this->_world->setBlock(glm::ivec3(17, this->_world->getTerrainHeight(17.5, 17.5), 17) + glm::ivec3(dx, dy, dz), this->_blocks["amethyst"]);
            }
        }
    }

    // // Place collision test
    // for(int dz = 0; dz < 9; dz++) {
    //     if(dz < 3 || dz > 5) {
    //         this->_world->setBlock(glm::vec3(34.0f, this->_world->getTerrainHeight(34.0f, 5.0f), 5.0f + dz), this->_block_planks);
    //     }
    //     this->_world->setBlock(glm::vec3(34.0f, this->_world->getTerrainHeight(34.0f, 5.0f) + 1.0f, 5.0f + dz), this->_block_planks);
    // }

    // Scatter mushrooms
    for(size_t i = 0; i < 150; i++) {
        const int x = f8::randi(0, 4*Chunk::CHUNK_SIZE, state), z = f8::randi(0, 4*Chunk::CHUNK_SIZE, state);
        const glm::vec3 pos = glm::vec3(x, this->_world->getTerrainHeight(x + 0.5f, z + 0.5f), z);
        // Check if ground mostly flat and empty
        if(this->_world->getTerrainHeight(pos.x, pos.z) - glm::floor(pos.y) < 0.125 && !this->_world->getBlock(pos)) {
            this->_world->setBlock(pos, this->_blocks["mushroom"]);
        }
    }

    // Scatter tall grass
    for(size_t i = 0; i < 350; i++) {
        const int x = f8::randi(0, 4*Chunk::CHUNK_SIZE, state), z = f8::randi(0, 4*Chunk::CHUNK_SIZE, state);
        const glm::vec3 pos = glm::vec3(x, this->_world->getTerrainHeight(x + 0.5f, z + 0.5f), z);
        // Check if ground mostly flat and empty
        if(this->_world->getTerrainHeight(pos.x, pos.z) - glm::floor(pos.y) < 0.1875 && !this->_world->getBlock(pos)) {
            this->_world->setBlock(pos, this->_blocks["tall_grass"]);
        }
    }

    // Setup players
    this->_player = get_player(*this->_world, *this->_shaders.primary, *this->_tm, this->_player_name);
    this->_player->setPosition({32.0f, 0.0f, 32.0f});
    this->_player->setHealth(this->_player->getMaxHealth());

    this->_macguffin = std::make_shared<MacGuffin>(*this->_world);
    this->_world->add(this->_macguffin);
    this->_macguffin->scatter();

    this->add_zombie();
}

/// Summons a new zombie
void MCEngine::add_zombie(const glm::vec3 pos) {
    const std::shared_ptr<Zombie> zombie = std::make_shared<Zombie>(*this->_world, *this->_shaders.primary, std::array<GLuint, 2> {this->_tm->load("assets/textures/entity/zombie.png"), this->_tm->load("assets/textures/dull.png")});
    zombie->setTarget(this->_player);
    zombie->setHealth(zombie->getMaxHealth());
    zombie->setPosition(pos);
    this->_world->add(zombie);
}


/// Generates a tree at pos with variable log height
void MCEngine::_place_tree(const glm::ivec3 pos, const size_t height) {
    for(int dy = 0; dy <= height; dy++) {
        this->_world->setBlock(pos + glm::ivec3(0, dy, 0), dy == height ? this->_blocks["leaves"] : this->_blocks["log"]);
        
        if(dy > height - 2) {
            for(int dx = -1; dx <= 1; dx++) {
                for(int dz = -1; dz <= 1; dz++) {
                    if((dx == 0) ^ (dz == 0)) {
                        this->_world->setBlock(pos + glm::ivec3(dx, dy, dz), this->_blocks["leaves"]);
                    }
                }
            }
        } else if(dy > height - 4) {
            for(int dx = -2; dx <= 2; dx++) {
                for(int dz = -2; dz <= 2; dz++) {
                    if((dx != 0) || (dz != 0)) {
                        this->_world->setBlock(pos + glm::ivec3(dx, dy, dz), this->_blocks["leaves"]);
                    }
                }
            }
        }
    }
}

void MCEngine::mSetupScene() {
    this->_freecam = std::make_shared<CSCI441::FreeCam>((GLfloat)mWindowWidth / (GLfloat)mWindowHeight, 45.0f, 0.001f, 1000.0f);
    this->_freecam->setPosition(glm::vec3(-4.0f, 4.0f, -4.0f));
    this->_freecam->setTheta(-1.25f*M_PI);
    this->_freecam->setPhi(M_PI / 2.8f);
    this->_freecam->recomputeOrientation();
    
    this->_fixedcam = std::make_shared<CSCI441::FixedCam>();
    
    // All other cameras are handled by the Player

    _lastTime = (GLfloat)glfwGetTime();
}

/*** Engine Cleanup ***/
void MCEngine::mCleanupShaders() {
    fprintf(stdout, "[INFO]: ...deleting Shaders.\n");
    this->_shaders = Shaders {};
    this->_shader_globals = nullptr;
}

void MCEngine::mCleanupBuffers() {
    fprintf(stdout, "[INFO]: ...deleting VAOs....\n");
    this->_world = nullptr;
    this->_grid = nullptr;
    this->_skybox = nullptr;
    this->_clouds = nullptr;
    this->_player = nullptr;
    this->_macguffin = nullptr;
    this->_blocks.clear();
    this->_pr = nullptr;
}

void MCEngine::mCleanupTextures() {
    fprintf(stdout, "[INFO]: ...deleting textures\n");
    this->_tm = nullptr;
}

void MCEngine::mCleanupScene() {
    fprintf(stdout, "[INFO]: ...deleting scene...\n");
    this->_freecam = nullptr;
    this->_fixedcam = nullptr;
}

/*** Camera Utility ***/

CSCI441::Camera* MCEngine::getPrimaryCamera() const {
    switch(this->_primaryCamera) {
        case 2:
            return this->_fixedcam.get();
        case 1:
            return &this->_player->getArcballCamera();
        case 0:
        default:
            return this->_freecam.get();
    }
}

glm::vec2 MCEngine::getPrimaryCameraRotationScale() const {
    switch(this->_primaryCamera) {
        case 1:
            return glm::vec2(1.0f, -1.0f);
        case 0:
        default:
            return glm::vec2(1.0f, 1.0f);
    }
}

CSCI441::Camera* MCEngine::getSecondaryCamera() const {
    switch(this->_secondaryCamera) {
        case 1:
            return &this->_player->getFirstPersonCamera();
        case 2:
            return &this->_player->getSkyCamera();
        case 0:
        default:
            return nullptr;
    }
}

/// Reads a line of text from stdin without blocking, returns empty string if no input
std::string readasync() {
    static std::string buffer;
    char c;
   
    while(std::cin.readsome(&c, 1)) {
        switch(c) {
            case '\n': return std::move(buffer);
            default: buffer.push_back(c);
        }
    }

    return "";
}

/// Read debug commands from terminal
void MCEngine::_handleConsoleInput() {
    std::string line = readasync();
    std::istringstream stream(line);
    std::string cmd;
    stream >> cmd;

    if(!cmd.size() || cmd.rfind('#', 0) == 0) {
        
    } else if(cmd == "exit") {
        this->setWindowShouldClose();
    } else if(cmd == "tp") {
        float x, y, z;
        stream >> x >> y >> z;

        if(std::cin.fail()) {
            fprintf(stderr, "[ERROR]: Invalid position\n");
        } else {
            this->_player->setPosition(glm::vec3(x, y, z));
        }
    } else if(cmd == "setblock") {
        float x, y, z;
        std::string name;
        stream >> x >> y >> z;
        stream >> name;

        const auto block = this->_blocks.find(name);

        if(std::cin.fail() || block == this->_blocks.end()) {
            fprintf(stderr, "[ERROR]: Invalid position or block\n");
        } else {
            this->_world->setBlock(glm::vec3(x, y, z), block->second);
        }
    } else if(cmd == "summon") {
        std::string name;
        stream >> name;

        if(name == "zombie") {
            this->add_zombie();
        } else {
            fprintf(stderr, "[ERROR]: Unknown creature\n");
        }
    } else if(cmd =="unstuck") {
        this->_player->setPosition({32.0f, 0.0f, 32.0f});
        this->_macguffin->scatter();
    } else if(f8::cyrb(cmd) == 18374542636879944076U) {
        // No hints for you unless you break the hash!
        fprintf(stdout, "Diamond is at %f, %f, %f\n", this->_macguffin->getPosition().x, this->_macguffin->getPosition().y, this->_macguffin->getPosition().z);
    } else if(cmd == "md5play") {
        stream >> std::ws;
        this->_movie = md5camera::load(std::string(std::istreambuf_iterator<char>(stream), {}).c_str());
    } else {
        fprintf(stderr, "[ERROR]: Unknown command '%s'\n", cmd.c_str());
    }

    if(line != "") {
        fprintf(stdout, "> ");
        fflush(stdout);
    }
}


/*** Rendering/Drawing Functions ***/

void MCEngine::_renderScene(glutils::RenderContext& ctx) const {
    this->_skybox->draw(ctx);
    this->_clouds->draw(ctx);

    ctx.bind(*this->_shaders.primary);
    this->_grid->draw(ctx);

    this->_player->draw(ctx);

    this->_world->draw(ctx);
}

void MCEngine::_renderUI(glutils::RenderContext& ctx) const {
    const float em = 1.0f/32.0f;

    // Render deaths
    this->_pr->sprite("\x82", {0.0f, 4.0f*em, 0.0f}, glutils::hex3(0xffffff), 2.0f*em);
    this->_pr->sprite("x", {2.0f*em, 4.0f*em, 0.0f}, glutils::hex3(0x0), 2.0f*em);
    this->_pr->sprite(std::to_string(this->_deaths), {4.0f*em, 4.0f*em, 0.0f*em}, glutils::hex3(0x0), 2.0f*em);

    // Render score
    this->_pr->sprite("\x89", {0.0f, 2.0f*em, 0.0f}, glutils::hex3(0x00a3f4), 2.0f*em);
    this->_pr->sprite("x", {2.0f*em, 2.0f*em, 0.0f}, glutils::hex3(0x0), 2.0f*em);
    this->_pr->sprite(this->_score == -1 ? "\x9a" : std::to_string(this->_score), {4.0f*em, 2.0f*em, 0.0f}, glutils::hex3(0x0), 2.0f*em);

    // Render health
    for(int i = 0; i < this->_player->getHealth(); i++) {
        this->_pr->sprite("\xc0", {2.0f*em*i + em/4.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 2.0f*em);
    }

    if(this->_debug) {
        // Because of z buffer, draw front to back
        glm::vec2 area = this->_pr->sprite(glutils::format("XYZ:%.1f,%.1f,%.1f", this->_player->getPosition().x, this->_player->getPosition().y, this->_player->getPosition().z), {0.0f, 1.0f - em, 0.0f}, glm::vec3(1.0f, 1.0f, 1.0f), em);
        this->_pr->rect({0.0f, 1.0f - em}, area, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
        
        area = this->_pr->sprite(glutils::format("CHK:%d,%d,%d", Chunk::getChunkPos(this->_player->getPosition()).x, Chunk::getChunkPos(this->_player->getPosition()).y, Chunk::getChunkPos(this->_player->getPosition()).z), {0.0f, 1.0f - 2.0f*em, 0.0f}, glm::vec3(1.0f, 1.0f, 1.0f), em);
        this->_pr->rect({0.0f, 1.0f - 2.0f*em}, area, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));

        area = this->_pr->sprite(glutils::format("GND:%.1f", this->_world->getTerrainHeight(_player->getPosition().x, this->_player->getPosition().z)), {0.0f, 1.0f - 3.0f*em, 0.0f}, glm::vec3(1.0f, 1.0f, 1.0f), em);
        this->_pr->rect({0.0f, 1.0f - 3.0f*em}, area, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    }
}

void MCEngine::_updateScene() {
    GLfloat currTime = (GLfloat) glfwGetTime();
    GLfloat deltaTime = currTime - _lastTime;
    this->_lastTime = currTime;

    this->_shader_globals->setUniform("time", currTime);

    // Update keybinds
    this->_im->poll(this->mpWindow, deltaTime);
    
    // Update players
    // Despite being an entity, the player is handled seperatly from the rest stored in the world
    this->_player->update(deltaTime);
    
    this->_world->update(deltaTime);

    if(this->_macguffin->isTouching(*this->_player)) {
        this->_score++;
        this->_macguffin->scatter();

        // Summon a new zombie sometimes
        if(f8::randb(0.33f)) {
            this->add_zombie();
        }
    }

    if(this->_player->getHealth() <= 0) {
        this->_player->setPosition({32.0f, 0.0f, 32.0f});
        this->_player->setHealth(this->_player->getMaxHealth());
        this->_deaths++;
    }


    // Play movie
    if(!this->_movie.empty()) {
        const md5camera::CameraConfig frame = this->_movie.front();
        this->_movie.pop();

        this->_primaryCamera = frame.primaryCamera;
        this->_secondaryCamera = frame.secondaryCamera;
        this->_fixedcam->setPosition(frame.eyePos);
        this->_fixedcam->setLookAtPoint(frame.eyePos + frame.camDir);
        this->_fixedcam->setUpVector(frame.upVec);
        this->_fixedcam->setVerticalFOV(frame.fov);
        this->_fixedcam->computeViewMatrix();
    }
}

void MCEngine::run() {
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

        // Read console commands
        _handleConsoleInput();

        // Update the scene
        _updateScene();

        // Draw primary camera to the whole window
        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glutils::RenderContext ctx(*this->_pr, this->_debug);
        this->_shader_globals->setUniform("projection", this->getPrimaryCamera()->getProjectionMatrix());
        this->_shader_globals->setUniform("view", this->getPrimaryCamera()->getViewMatrix());
        this->_shader_globals->setUniform("eyePos", this->getPrimaryCamera()->getPosition());
        _renderScene(ctx);
        _renderUI(ctx);

        // Optional secondary cameras
        if(this->getSecondaryCamera() != nullptr) {
            this->_pr->rect({0.745f, 0.745}, {0.255f, 0.255f}, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));

            // Clear just the region of the mini-viewport
            glEnable(GL_SCISSOR_TEST);
            glScissor(framebufferWidth*0.75, framebufferHeight*0.75, framebufferWidth*0.25, framebufferHeight*0.25);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_SCISSOR_TEST);
            
            // Draw secondary camera
            glViewport(framebufferWidth*0.75, framebufferHeight*0.75, framebufferWidth*0.25, framebufferHeight*0.25);
            glutils::RenderContext ctx(*this->_pr, this->_debug);
            this->_shader_globals->setUniform("projection", this->getSecondaryCamera()->getProjectionMatrix());
            this->_shader_globals->setUniform("view", this->getSecondaryCamera()->getViewMatrix());
            this->_shader_globals->setUniform("eyePos", this->getSecondaryCamera()->getPosition());
            
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
input::InputManager& MCEngine::getInputManager() {
    return *this->_im;
}

/*** Callbacks (forward to input manager) ***/
void keyboard_callback(GLFWwindow *window, const int key, const int scancode, const int action, const int mods) {
    auto engine = static_cast<MCEngine*>(glfwGetWindowUserPointer(window));
    if(key != GLFW_KEY_UNKNOWN) {
        if(action == GLFW_PRESS) {
            engine->getInputManager().dispatch(window, input::key(key), input::State::Press);
        } else if(action == GLFW_RELEASE) {
            engine->getInputManager().dispatch(window, input::key(key), input::State::Release);
        }
    }
}

void cursor_callback(GLFWwindow *window, const double x, const double y) {
    auto engine = static_cast<MCEngine*>(glfwGetWindowUserPointer(window));
    engine->getInputManager().cursor(glm::vec2(x, engine->getWindowHeight() - y));
}

void mouse_button_callback(GLFWwindow *window, const int button, const int action, const int mods) {
    auto engine = static_cast<MCEngine*>(glfwGetWindowUserPointer(window));
    if(action == GLFW_PRESS) {
        engine->getInputManager().dispatch(window, input::mouse(button), input::State::Press);
    } else if(action == GLFW_RELEASE) {
        engine->getInputManager().dispatch(window, input::mouse(button), input::State::Release);
    }
}

void scroll_callback(GLFWwindow *window, const double xOffset, const double yOffset) {
    const auto engine = static_cast<MCEngine*>(glfwGetWindowUserPointer(window));
    engine->getInputManager().scroll(glm::vec2(xOffset, yOffset));
}
