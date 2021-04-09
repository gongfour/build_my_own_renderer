#pragma once
#include "vector3.hpp"

struct Sphere
{
    Vec3f center;
    float radius, radius2;
    Vec3f surfaceColor, emissionColor;
    float transparency, reflection;
    Sphere(
        const Vec3f &c,
        const float &r,
        const Vec3f &sc,
        const float &refl = 0,
        const float &transp = 0,
        const Vec3f &ec = 0)
        : center(c), radius(r), radius2(r*r), surfaceColor(sc), emissionColor(ec)
        , transparency(transp), reflection(refl)
    {}

    bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const
    {
        Vec3f l = center - rayorig;
        float tca = l.dot(raydir); // ray 방향으로의 성분
        if (tca < 0) return false; // ray 방향이 반대인 경우

        // l^2 < tca^2 + r^2 (1)
        // l^2 == tca^2 + r^2 이면 ray가 sphere의 접선인 경우
        // (1) 조건을 만족한다면 ray는 구를 교차하여 2개의 교차점이 생기게 된다.
        // d는 교차하는 두점을 잇는 선분과 구의 중심점 사이의 거리이다.
        float d2 = l.dot(l) - tca * tca;
        if (d2 > radius2) return false; 

        // thc: '교차하는 두 점을 잇는 선분의 길이/2'를 계산한 것
        float thc = sqrt(radius2 - d2);
        t0 = tca - thc; // 첫번째 교차점 까지의 길이
        t1 = tca + thc; // 두번째 교차점 까지의 길이
        return true;
    }
};