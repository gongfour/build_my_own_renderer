#pragma once
#include "vector3.hpp"
#include "sphere.hpp"
#include "trace.hpp"
#include <vector>
#include <fstream>

const auto M_PI = 3.141592653589793L;

void render(const std::vector<Sphere> &spheres)
{
    unsigned width = 640, height = 480;
    Vec3f *image = new Vec3f[width*height];
    Vec3f *pixel = image;
    float invWidth = 1 / float(width), invHeight = 1 / float(height);
    float fov = 30, aspectratio = width / float(height);
    float angle = tan(M_PI*0.5*fov/180.);

    for (unsigned y=0; y<height; ++y) {
        for(unsigned x=0; x<width; ++x, ++pixel) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio; 
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle; 
            Vec3f raydir(xx,yy,-1); // 빛은 들어오는 방향
            raydir.normalize(); 
            *pixel = trace(Vec3f(0), raydir, spheres, 0); 
        }
    }

    std::ofstream ofs("./my-own-render.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (unsigned i = 0; i < width * height; ++i) {
        ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
               (unsigned char)(std::min(float(1), image[i].y) * 255) <<
               (unsigned char)(std::min(float(1), image[i].z) * 255);
    }
    ofs.close();
    delete [] image;
}