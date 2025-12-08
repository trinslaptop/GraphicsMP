// Ad hoc enum included on both the GLSL sprite.g.glsl and C++ PrimitiveRenderer.hpp sides for controling sprite rendering mode
// GLSL wraps this include with a `const int` and `;` while C++ wraps it in an enum

/// Renders a sprite flat on the UI centered around the given position (ignores z pos)
UI_ANCHOR_CENTER = 0,

/// Renders a sprite flat on the UI with it's lower left corner at the given position (ignores z pos)
UI_ANCHOR_CORNER = 1,

/// Renders a billboarded sprite in the 3D world 
PARTICLE = 2,

/// Computes size without actually drawing sprite
HIDDEN = 3