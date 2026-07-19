#pragma once

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
#include <string>
#include <sstream>
#include <charconv>

extern "C" {
    #include "../cubiomes-viewer-bedrock/cubiomes/finders.h"
}

class Spiral {
    int x = 0, z = 0;
    int dx = 0, dz = -1;
    int stepSize = 1, stepsTaken = 0, turns = 0;

public:
    std::pair<int,int> next() {
        std::pair<int,int> current = {x, z};

        x += dx;
        z += dz;
        stepsTaken++;

        if (stepsTaken == stepSize) {
            stepsTaken = 0;

            int temp = dz;
            dz = -dx;
            dx = temp;

            turns++;
            if (turns % 2 == 0) stepSize++;
        }

        return current;
    }
};

namespace rev {
    const int64_t REV_REGION_A = 341873128712L;
    const int64_t REV_REGION_B = 132897987541L;
    const int64_t REV_SALT = 10387312L;

    int mod(int a, int m) {
        int r = a % m;
        return (r < 0) ? r + m : r;
    }

    uint32_t getVillageRegionSeed(int worldSeed, int chunkX, int chunkZ) {
        int regX = chunkX < 0 ? chunkX - 39 : chunkX;
        int regZ = chunkZ < 0 ? chunkZ - 39 : chunkZ;

        return regX * REV_REGION_A + regZ * REV_REGION_B + worldSeed + REV_SALT;
    }

    int getVillageWorldSeed(uint32_t regionSeed, int chunkX, int chunkZ) {
        int regX = chunkX < 0 ? chunkX - 39 : chunkX;
        int regZ = chunkZ < 0 ? chunkZ - 39 : chunkZ;

        int64_t C = regX * REV_REGION_A + regZ * REV_REGION_B + REV_SALT;
        return regionSeed - (uint32_t)C;
    }

    uint32_t getStrongholdRegionSeed(uint32_t worldSeed, int chunkX, int chunkZ) {
        return setPopulationSeed(worldSeed, chunkX, chunkZ);
    }

    bool doesVillageSpawn(int regionSeed, int chunkX, int chunkZ, int *worldSeedOut) {
        // i think this is correct logic
        setSeed(regionSeed);
        int startX = nextInt(28);
        int startZ = nextInt(28);
        if (mod(chunkX, 40) != startX || mod(chunkZ, 40) != startZ) {
            return false;
        }

        int worldSeed = getVillageWorldSeed(regionSeed, chunkX, chunkZ);

        Generator g;
        setupGenerator(&g, MC_1_2, 0);
        applySeed(&g, DIM_OVERWORLD, (uint32_t)worldSeed);

        Pos village = { chunkX * 16 + 4, chunkZ * 16 + 4 };
        if (isViableStructurePos(Village, &g, village.x, village.z, 0)) {
            *worldSeedOut = worldSeed;
            return true;
        } else {
            return false;
        }
    }
}

class VillageIter {
    Spiral spiral;
    uint32_t regionSeed;
    int startX;
    int startZ;

public:
    VillageIter(uint32_t regionSeed) {
        this->regionSeed = regionSeed;
        setSeed(regionSeed);
        startX = nextInt(28);
        startZ = nextInt(28);
    }

    void skip(uint32_t n) {
        for (uint32_t i=0; i<n; i++) {
            spiral.next();
        }
    }

    // returns world seed, modifies pos
    int nextCandidate(Pos *pos) {
        auto [x, z] = spiral.next();
        Pos chunk = { x * 40 + startX, z * 40 + startZ };
        pos->x = chunk.x * 16 + 4;
        pos->z = chunk.z * 16 + 4;
        return rev::getVillageWorldSeed(regionSeed, chunk.x, chunk.z);
    }

    int nextSpawn(Pos *pos) {
        while (true) {
            int worldSeed = nextCandidate(pos);

            Generator g;
            setupGenerator(&g, MC_1_2, 0);
            applySeed(&g, DIM_OVERWORLD, (uint32_t)worldSeed);

            if (isViableStructurePos(Village, &g, pos->x, pos->z, 0)) {
                return worldSeed;
            }
        }
    }
};

namespace coords {
    Pos pieceOffset(Piece &p, int d1, int d2) {
        Pos out = {};

        // i dont even know which rotation is which
        switch (p.rot) {
            case 0:
                out.x = p.bb0.x + d1;
                out.z = p.bb0.z + d2;
                break;
            case 1:
                out.x = p.bb1.x - d2;
                out.z = p.bb0.z + d1;
                break;
            case 2:
                out.x = p.bb0.x + d1;
                out.z = p.bb1.z - d2;
                break;
            case 3:
                out.x = p.bb0.x + d2;
                out.z = p.bb0.z + d1;
                break;
        }

        return out;
    }

    double dist(Pos p1, Pos p2) {
        double dx = p1.x - p2.x;
        double dz = p1.z - p2.z;
        return sqrt(dx*dx + dz*dz);
    }

    // lalala clone
    double dist(Pos3 p1, Pos3 p2) {
        double dx = p1.x - p2.x;
        double dz = p1.z - p2.z;
        return sqrt(dx*dx + dz*dz);
    }

    double angle(Pos a, Pos b, Pos c) {
        double ax = a.x - b.x, az = a.z - b.z;
        double cx = c.x - b.x, cz = c.z - b.z;
        double d = (ax * cx + az * cz) / (std::hypot(ax, az) * std::hypot(cx, cz));
        return std::acos(std::clamp(d, -1.0, 1.0));
    }
}

namespace str {
    std::vector<std::string> split(std::string input, char c = ',') {
        std::stringstream ss(input);

        std::string token;
        std::vector<std::string> parts;

        while (std::getline(ss, token, c)) {
            if (token.front() == '"') token.erase(0, 1);
            if (token.back() == '"') token.pop_back();
            parts.push_back(token);
        }

        return parts;
    }

    // just assumes it works fine
    int64_t num(std::string input) {
        int64_t value = 0;
        std::from_chars(input.data(), input.data() + input.size(), value);
        return value;
    }
};
