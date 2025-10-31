/*
 *  CSCI 441, Computer Graphics, Fall 2025
 *
 *  Project: A3
 *  File: main.cpp
 *
 *  Description: Basic entry point for the assignment, not much here
 *
 *  Author: Trin Wasinger, 2025
 *  Original Author: Dr. Paone, Colorado School of Mines, 2025
 *
 */

#define CSCI441_SCREENSHOT_IMPLEMENTATION
#include "MPEngine.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Our main function
int main() {
    const auto engine = new A3Engine();
    engine->initialize();
    if (engine->getError() == CSCI441::OpenGLEngine::OPENGL_ENGINE_ERROR_NO_ERROR) {
        engine->run();
    }
    engine->shutdown();
    delete engine;
	return EXIT_SUCCESS;
}
