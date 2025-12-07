#ifndef SHADERPROGRAM_HPP
#define SHADERPROGRAM_HPP

#include "include/mcpre.hpp"
#include <string>

/*
 * ShaderProgram.hpp
 * Trin Wasinger - Fall 2025
 *
 * This shims CSCI441/ShaderUtils.hpp and CSCI441/ShaderProgram to add support for
 * `#include "..."` and other preprocessor stuff. Here be dragons!
 */

/////////////////////////////////////////
// <!> CAUTION: DIRTY HACKS AHEAD! <!> //
/////////////////////////////////////////

// Include CSCI441/ShaderUtils.hpp but overwrite compileShader()
#define compileShader _compileShader
#include <CSCI441/ShaderUtils.hpp>
#undef compileShader

// Provide a shimmed compileShader()
// It's mostly the same except that it add a preprocessor for `#include "..."`
namespace CSCI441_INTERNAL::ShaderUtils {
    /// Reads the contents of a text file and compiles the associated shader type, returns 0 on error
    inline GLuint compileShader(const char *filename, GLenum shaderType) {
        // Will hold contents of shader source code, loaded from file
        std::string shaderString;

        // Read in and preprocess text file
        if(!(shaderString = mcpre::preprocess(filename, {}, {"include", "pragma"})).empty()) {
            // Generate a shader handle for the corresponding shader type
            const GLuint shaderHandle = glCreateShader(shaderType);

            // Send the contents of each program to the GPU
            const char* cstr = shaderString.c_str();
            glShaderSource(shaderHandle, 1, &cstr, nullptr);

            // Compile each shader on the GPU
            glCompileShader(shaderHandle);

            // Check the shader log
            printShaderLog(shaderHandle);

            // Return the handle of our shader
            return shaderHandle;
        } else {
            return 0;
        }
    }
}

// Now include CSCI441/ShaderProgram.hpp. Since CSCI441/ShaderUtils.hpp is included
// above and it has a proper include guard, it won't get included again, and
// CSCI441/ShaderProgram.hpp will use this compileShader() shim instead of the original
#include <CSCI441/ShaderProgram.hpp>

// Provide a convenient typedef
typedef CSCI441::ShaderProgram ShaderProgram;

#endif