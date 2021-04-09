#pragma once
#include <vector>
#include <cassert>
#include "vector3.hpp"
#include "sphere.hpp"

const auto MAX_RAY_DEPTH = 5;

// a와 b를 mix 비율에 따라 섞어줌
float mix(const float &a, const float &b, const float &mix)
{
    return b * mix + a * (1 - mix);
}

Vec3f trace(
    const Vec3f &rayorig,
    const Vec3f &raydir,
    const std::vector<Sphere> &spheres,
    const int &depth)
{
    // assert(raydir.length() == 1);
    float tnear = INFINITY;
    const Sphere* sphere = nullptr;

    // 가장 가까운 t0값 찾기
    // 가장 가까운 sphere 찾기
    for (unsigned i=0; i<spheres.size(); ++i) {
        float t0 = INFINITY, t1 = INFINITY;
        if (spheres[i].intersect(rayorig, raydir, t0, t1)) {
            if (t0 < 0) t0 = t1;
            if (t0 < tnear) {
                tnear = t0;
                sphere = &spheres[i];
            }
        }
    }
    if (!sphere) return Vec3f(2); // 못 찾았을 경우
    Vec3f surfaceColor = 0;
    Vec3f phit = rayorig + raydir * tnear; // 카메라에서 가장 가까운 교차점
    Vec3f nhit = phit - sphere->center; // 교차점의 normal vector
    nhit.normalize();

    float bias = 1e-4; // 편차
    bool inside = false;
    /*
    만약 raydir과 nhit이 서로 반대방향이 아니라면, 카메라가 구체 내에 있다는 말임
        따라서, inside를 true로 변경하고, nhit의 부호를 바꿔줌         
    */
    if (raydir.dot(nhit) > 0) {
        nhit = -nhit;
        inside = true;
    }
    
    if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH) {
        float facingratio = -raydir.dot(nhit); // 정면으로 부딪치는 성분의 비
        // 굴절률이 다른 매질을 투과할 때 반사와 굴절이 일어나는데, 그 성분을 나누어 분석하는 방정식.
        // 이 방정식을 통해서 반사율과 투과율을 계산 할 수 있다.
        float fresneleffect = mix(pow(1-facingratio, 3), 1, 0.1);
        Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit); // 반사방향
        refldir.normalize();
        // 재귀적으로 동작
        // todo rayorig : phit + nhit * bias 
        Vec3f reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1);
        Vec3f refraction = 0; // 굴절률

        if (sphere->transparency) { // 확산(transmission)

            float ior = 1.1;
            // 안에서 밖으로 나가는지? 밖에서 안으로 들어오는지에 따라 굴절률이 역수가 됨
            float eta = (inside) ? ior : 1 / ior;
            float cosi = -nhit.dot(raydir); // 입사각
            // snell's law : 굴절률에 관한 법칙
            // 굴절벡터를 알아낸다.
            float k2 = 1 - eta*eta*(1-cosi*cosi);
            Vec3f refrdir = raydir * eta + nhit * (eta*cosi-sqrt(k2));
            refrdir.normalize();
            refraction = trace(phit - nhit*bias, refrdir, spheres, depth+1);
        }

        surfaceColor = sphere->surfaceColor * 
            (reflection * fresneleffect + 
            refraction * (1 - fresneleffect) * sphere->transparency);
    }
    else { // diffuse object이므로 더 이상 raytrace를 수행할 필요가 없음
        for (unsigned i=0; i<spheres.size(); ++i) {
            if (spheres[i].emissionColor.x > 0) {
                // emissionColor: 물체가 자체가 빛 발함
                // 즉, 이 조건을 통과하면 빛임.
                Vec3f transmission = 1;
                // 빛 물체(spheres[i])가 존재하는 방향
                Vec3f lightDirection = spheres[i].center - phit;
                lightDirection.normalize();
                
                // i index는 빛 물체를 가르킴
                // j index는 어떠한 물체를 가르킴
                // 빛 물체로 가는 길에 j 물체가 가로막고 있으면 transmission 값을 0으로 변경
                for(unsigned j=0; j<spheres.size(); ++j) {
                    if (i != j) {
                        float t0, t1;
                        if (spheres[j].intersect(phit + nhit*bias, lightDirection, t0, t1)) {
                            transmission = 0;
                            break;
                        }
                    }
                }
                // 해당하는 빛 물체에 의해 생기는 surfaceColor 값을 누적
                // 빛 물체가 가로 막혀 있다면 0 (transmission)
                // 빛을 등지고 있으면 0 (그림자 영역을 말함)
                // spheres[i].emissionColor (빛의 세기)
                surfaceColor += sphere->surfaceColor * transmission *
                    std::max(float(0), nhit.dot(lightDirection)) * spheres[i].emissionColor;
            }
        }
    }

    // 결과적으로 surfacecolor를 얻어내기 위한 과정이다.
    return surfaceColor + sphere->emissionColor;
}