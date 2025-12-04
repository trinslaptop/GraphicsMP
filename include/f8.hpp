#ifndef F8_HPP
#define F8_HPP

#include <stdint.h>
#include <ctime>

#include <glm/glm.hpp>

/// F8 (c) Trin Wasinger 2025
///
/// F8 ("Fate") is a machine independent PRNG library with support for stateful
/// random numbers via xorshift32 and positional random numbers with FBM.
///
/// Partially based on https://thebookofshaders.com/13/
///
/// NOTE: F8's global state is per include!
namespace f8 {
    namespace {
        uint32_t _state = 0x80801;
        glm::vec2 _vstate = glm::vec2(0.0f);
    }

    /// Get a random uint32_t
    inline uint32_t rand(uint32_t& state = _state) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }

    /// Get a random integer in [min, max)
    inline uint32_t randi(const int min, const int max, uint32_t& state = _state) {
        return rand(state) % (max - min) + min;
    }

    /// Get a random float in [0.0f, 1.0f)
    inline float randf(uint32_t& state = _state) {
        return rand(state) / glm::pow(2, 32);
    }

    /// Sets the RNG seed to the given value or the current time if argument is absent (or 0)
    /// and returns the new seed, also cycles the generator a few times
    inline unsigned int srand(const unsigned int seed = 0, uint32_t& state = _state) {
        state = seed ? seed : time(NULL);
        
        for(size_t i = 0; i < 16; i++) {
            rand(state);
        }
        
        return seed;
    }

    /// Sets the vector RNG seed to the given value or the current time if argument is absent (or 0)
    inline const glm::vec2 srandv(const unsigned int seed = 0, glm::vec2& vecseed = _vstate) {
        uint32_t t = seed ? seed : time(NULL);
        const float a = randf(t), b = randf(t);
        return vecseed = glm::vec2(a, b);
    }

    /// Get a random float in [0.0f, 1.0f] from a vector state
    inline float randfv(const glm::vec2& v, const glm::vec2& state = _vstate) {
        return glm::fract(glm::sin(glm::dot(v + state, glm::vec2(12.9898f,78.233f)))*43758.5453123f);
    }

    /// Get a smoothly changing random float in [0.0f, 1.0f] from a vector state
    inline float noise(const glm::vec2& v, const glm::vec2& state = _vstate) {
        const glm::vec2 i = glm::floor(v);
        const glm::vec2 f = glm::fract(v);

        // Four corners in 2D of a tile
        const float a = randfv(i, state);
        const float b = randfv(i + glm::vec2(1.0f, 0.0f), state);
        const float c = randfv(i + glm::vec2(0.0f, 1.0f), state);
        const float d = randfv(i + glm::vec2(1.0f, 1.0f), state);

        const glm::vec2 u = f*f*(3.0f - 2.0f*f);

        return glm::mix(a, b, u.x) + (c - a)*u.y*(1.0f - u.x) + (d - b)*u.x*u.y;
    }

    /// Get a random float in [0.0f, 1.0f] via Fractal Brownian Motion
    inline float fbm(glm::vec2 v, const uint8_t octaves = 6, const glm::vec2& state = _vstate) {
        float value = 0.0f;
        float amplitude = 0.5f;

        // Loop over octaves
        for (uint8_t i = 0; i < octaves; i++) {
            value += amplitude*noise(v, state);
            v *= 2.0f;
            amplitude *= 0.5f;
        }
        return value;
    }
}

#endif