#pragma once

#include <cstdint>
#include <utility>


// gpt'd
class Spiral {
    int x = 0, z = 0;
    int dx = 1, dz = 0;
    int step_size = 1, steps_taken = 0, turns = 0;

public:
    std::pair<int,int> next() {
        std::pair<int,int> current = {x, z};

        x += dx;
        z += dz;
        steps_taken++;

        if (steps_taken == step_size) {
            steps_taken = 0;

            int temp = dx;
            dx = -dz;
            dz = temp;

            turns++;
            if (turns % 2 == 0) step_size++;
        }

        return current;
    }
};

namespace rev {
    const int64_t REV_REGION_A = 341873128712L;
    const int64_t REV_REGION_B = 132897987541L;
    const int64_t REV_SALT = 10387312L;

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
}
