/*
 *  CSCI 441, Computer Graphics, Fall 2025
 *
 *  Project: MP
 *  File: main.cpp
 *
 *  Description: Basic entry point for the assignment, not much here
 *
 *  Author: Trin Wasinger, 2025
 *  Original Author: Dr. Paone, Colorado School of Mines, 2025
 *
 */

// Need to include this first for the shim to work!
#include "ShaderProgram.hpp"

#define CSCI441_SCREENSHOT_IMPLEMENTATION
#include "MCEngine.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>
#include <fstream>
#include <sstream>
#include <ios>

#include "include/json.hpp"

// Launch engine
int main(int argc, char* argv[]) {
    const std::string player = argc > 1 ? argv[1] : "Idril";

    std::ifstream istream("data/config.json");
    istream.exceptions(std::ios_base::badbit);
    std::ostringstream ostream;
    ostream << istream.rdbuf();

    const auto engine = new MCEngine(json::cast::object(json::parse(ostream.str())), player);
    engine->initialize();
    if (engine->getError() == CSCI441::OpenGLEngine::OPENGL_ENGINE_ERROR_NO_ERROR) {
        engine->run();
    }
    engine->shutdown();
    delete engine;
	return EXIT_SUCCESS;
}
