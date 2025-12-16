// Ad hoc enum included on both the GLSL texshader.g.glsl and C++ glutils.hpp sides for controlling rendering mode
// GLSL wraps this include with a `const int` and `;` while C++ wraps it in an enum

/// Renders colored geometry, should output a color buffer and depth buffer
PRIMARY_PASS = 0,

/// Renders shadows, should output a depth buffer
SHADOW_PASS = 1,

/// Renders effects based on primary pass, should output a color buffer
POSTPROCESS_PASS = 2