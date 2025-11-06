#ifndef MD5CAMERA_HPP
#define MD5CAMERA_HPP

/*
 * md5camera.hpp
 * Trin Wasinger - Fall 2025
 *
 * Reads in a set of camera positions and vectors to create a "movie"
 */

#include <glm/glm.hpp>
#include <queue>
#include <fstream>
#include <cstdio>

#include <iostream>

namespace md5camera {
    /// A set of camera parameters for a given frame
    struct CameraConfig {
        unsigned int primaryCamera = 0, secondaryCamera = 0;
        const glm::vec3 eyePos, camDir, upVec;
        float fov;
    };

    typedef std::queue<CameraConfig> MD5Movie;

    /// Loads a camera track from a text file, format is:
    /// nlines
    /// eyePosX eyePosY eyePosZ camDirX camDirY camDirZ upVecX upVecY upVecZ fov
    /// eyePosX eyePosY eyePosZ camDirX camDirY camDirZ upVecX upVecY upVecZ fov
    /// ...
    inline MD5Movie load(const char* path) {
        MD5Movie result;

        std::ifstream stream(path);
        if(stream.is_open()) {
            float eyePosX, eyePosY, eyePosZ, camDirX, camDirY, camDirZ, upVecX, upVecY, upVecZ, fov;
            size_t n;
            stream >> n;
            for(size_t frame = 0; frame < n; frame++) {
                stream >> eyePosX >> eyePosY >> eyePosZ
                    >> camDirX >> camDirY >> camDirZ
                    >> upVecX  >> upVecY  >> upVecZ
                    >> fov;
                result.push(CameraConfig {
                    .primaryCamera = 2,
                    .secondaryCamera = 0,
                    .eyePos = glm::vec3(eyePosX, eyePosY, eyePosZ),
                    .camDir = glm::vec3(camDirX, camDirY, camDirZ),
                    .upVec = glm::vec3(upVecX, upVecY, upVecZ),
                    .fov = fov
                });
            }

            // Last frame to reset cameras
            result.push(CameraConfig {
                .primaryCamera = 1,
                .secondaryCamera = 1,
                .eyePos = glm::vec3(eyePosX, eyePosY, eyePosZ),
                .camDir = glm::vec3(camDirX, camDirY, camDirZ),
                .upVec = glm::vec3(0.0f, 1.0f, 0.0f),
                .fov = 45.0f
            });
        } else {
            fprintf(stderr, "[ERROR]: Could not read movie file \"%s\"\n", path);
        }

        return result;
    }
}

#endif