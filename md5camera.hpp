#ifndef MD5CAMERA_HPP
#define MD5CAMERA_HPP

#include <glm/glm.hpp>
#include <vector>
#include <cstdio>
#include <fstream>

// TODO

namespace md5camera {
    struct FrameConfig {
        const glm::vec3 eyePos, camDir, upVec;
        float fov;
    };

    typedef std::vector<FrameConfig> MD5Movie;

    inline MD5Movie load(const char* path) {
        std::ifstream stream("test.txt");
        if(!stream.is_open()) {
            return MD5Movie();
        }
        
        float eyePosX, eyePosY, eyePosZ, camDirX, camDirY, camDirZ, upVecX, upVecY, upVecZ, fov;
        size_t n;
        // std::cout << n << " frames" << std::endl;
        
        stream >> n;
        for(size_t frame = 0; frame < n; frame++) {
            stream >> eyePosX >> eyePosY >> eyePosZ
                >> camDirX >> camDirY >> camDirZ
                >> upVecX  >> upVecY  >> upVecZ
                >> fov;
                
            fprintf(stdout, "Frame %d: eyePos={%f, %f, %f}, camDir={%f, %f, %f}, upVec={%f, %f, %f}, fov=%f\n", n, eyePosX, eyePosY, eyePosZ, camDirX, camDirY, camDirZ, upVecX, upVecY, upVecZ, fov);
        }
    }
}

#endif