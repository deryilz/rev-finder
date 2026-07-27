#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>

extern "C" {
    #include "../cubiomes-patch/finders.h"
}

// for mod 2^32 only
uint32_t inv(uint32_t a) {
    uint32_t x = a;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    x *= 2 - a * x;
    return x;
}

struct Vector2D {
    int64_t x;
    int64_t z;

    double dot(Vector2D o) {
        return (double)x * (double)o.x + (double)z * (double)o.z;
    }

    double norm_sq() {
        return dot(*this);
    }

    Vector2D operator-(Vector2D o) { return {x - o.x, z - o.z}; }
    Vector2D operator+(Vector2D o) { return {x + o.x, z + o.z}; }
    Vector2D operator*(int64_t k) { return {x * k, z * k}; }
};

// todo: probably update everything to be snake_case like this
std::pair<Vector2D, Vector2D> lagrange_gauss(Vector2D v1, Vector2D v2) {
    while (true) {
        if (v2.norm_sq() < v1.norm_sq()) {
            std::swap(v1, v2);
        }

        double v1_norm = v1.norm_sq();
        if (v1_norm == 0.0) break;

        double proj = v1.dot(v2) / v1_norm;

        int64_t m = floor(proj + 0.5);
        if (m == 0) break;

        v2 = v2 - v1 * m;
    }
    return {v1, v2};
}

class GaussInfo {
public:
    int seed;
    // coefficients
    int a;
    int b;
    int a_inv;
    // obvious basis vectors
    Vector2D b1;
    Vector2D b2;
    // reduced vectors
    Vector2D v1;
    Vector2D v2;
    double det_inv;

    GaussInfo(int seed) {
        this->seed = seed;
        setSeed(seed);

        a = next() | 1;
        b = next() | 1;
        a_inv = inv(a);

        b1 = { 1ULL<<32, 0 };
        b2 = { -a_inv*b, 1 }; // there's overflow but it's already done mod M

        auto result = lagrange_gauss(b1, b2);
        v1 = result.first;
        v2 = result.second;
        det_inv = 1.0 / (double)(v1.x * v2.z - v1.z * v2.x);
    }
};

class GaussChunkIter {
    GaussInfo &info;
    int64_t min_c1 = INT64_MAX, max_c1 = INT64_MIN;
    int64_t min_c2 = INT64_MAX, max_c2 = INT64_MIN;
    int64_t i, j, d;
    Vector2D p;
    bool done = false;

    void advance() {
        if (done) return;

        j++;
        if (j > max_c2) {
            j = min_c2;
            i++;
            if (i > max_c1) {
                done = true;
            }
        }
    }
public:
    GaussChunkIter(GaussInfo &info, int target, int d = 625) : info(info) {
        this->d = d;

        // point that we know works
        p = {(info.seed ^ target) * info.a_inv, 0};

        Vector2D corners[4] = {{d, d}, {d, -d}, {-d, d}, {-d, -d}};
        for (Vector2D v : corners) {
            double dx = p.x - v.x;
            double dz = p.z - v.z;
            double c1 = (info.v2.z * dx - info.v2.x * dz) * info.det_inv;
            double c2 = (info.v1.x * dz - info.v1.z * dx) * info.det_inv;

            min_c1 = std::min(min_c1, (int64_t)ceil(c1));
            max_c1 = std::max(max_c1, (int64_t)floor(c1));
            min_c2 = std::min(min_c2, (int64_t)ceil(c2));
            max_c2 = std::max(max_c2, (int64_t)floor(c2));
        }

        i = min_c1;
        j = min_c2;
    }

    bool nextMatch(Pos *pos) {
        while (!done) {
            Vector2D n = p - info.v1 * i - info.v2 * j;
            advance();
            if (abs(n.x) <= d && abs(n.z) <= d) {
                pos->x = n.x * 16;
                pos->z = n.z * 16;
                return true;
            }
        }
        return false;
    }
};
