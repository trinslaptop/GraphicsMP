#ifndef AUDIOCONTEXT_HPP
#define AUDIOCONTEXT_HPP

#include "NonCopyable.hpp"

#include <string>

#ifdef USE_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <map>
#include <set>
#else
typedef unsigned int ALuint;
#endif

/// Loads, stores, plays, and releases sounds, requires compile flag
/// Create a source for each unique thing that should play sounds then delete when no longer needed
class AudioContext final : NonCopyable {
#ifdef USE_OPENAL
    private:
        std::map<std::string, ALuint> _buffers;
        std::set<ALuint> _sources;

    public:
        static constexpr bool ENABLED = true;

        inline AudioContext() {
            alutInit(nullptr, nullptr);
        }

        inline ~AudioContext() {
            for(const auto& source : this->_sources) {
                alSourceStop(source);
                alDeleteSources(1, &source);
            }
            
            for(const auto& buffer : this->_buffers) {
                alDeleteBuffers(1, &buffer.second);
            }
            
            alutExit();
        }

        /// Retrieves sound from cache or loads it from a file
        inline ALuint load(const std::string& name) {
            // Try find existing
            std::map<std::string, ALuint>::iterator iter = this->_buffers.find(name);
            if(iter != this->_buffers.end()) {
                return iter->second;
            }

            const ALuint buffer = alutCreateBufferFromFile(name.c_str());
            if(buffer == AL_NONE) {
                fprintf(stderr, "[ERROR]: Could not load audio \"%s\"\n", name.c_str());
                return 0;
            }

            this->_buffers[name] = buffer;
            return buffer;
        }

        inline ALuint createSource() {
            ALuint source;
            alGenSources(1, &source);
            this->_sources.insert(source);
            return source;
        }

        inline void deleteSource(const ALuint source) {
            alSourceStop(source);
            alDeleteSources(1, &source);
            this->_sources.erase(source);
        }

        inline void play(const ALuint sound, const ALuint source) const {
            alSourcei(source, AL_BUFFER, sound);
            alSourcePlay(source);
        }

        inline void stop(const ALuint source) const {
            alSourceStop(source);
        }
#else
    public:
        static constexpr bool ENABLED = false;

        AudioContext() = default;
        ~AudioContext() = default;

        inline ALuint load(const std::string& name) {
            return 0;
        }
        
        inline ALuint createSource() {
            return 0;
        }
        
        inline void deleteSource(const ALuint source) {}
        inline void play(const ALuint sound, const ALuint source) const {}
        inline void stop(const ALuint source) const {}
#endif
};
#endif