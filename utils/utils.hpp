#include <cstdint>
#include <utility>

extern "C" {
    #include "../cubiomes-viewer-bedrock/cubiomes/finders.h"
}

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

    bool findInChunk(int regionSeed, int x, int z, int *out) {
        setSeed(regionSeed);
        if (!isVillageChunkNoSet(x, z)) return false;

        int worldSeed = getVillageWorldSeed(regionSeed, x, z);

        Generator g;
        setupGenerator(&g, MC_1_2, 0);
        applySeed(&g, DIM_OVERWORLD, (uint32_t)worldSeed);

        Pos village = { x * 16 + 4, z * 16 + 4 };
        if (isViableStructurePos(Village, &g, village.x, village.z, 0)) {
            *out = worldSeed;
            return true;
        } else {
            return false;
        }
    }

    // spiral strategy
    int findClosestViable(int regionSeed, Pos *pos) {
        Spiral s;
        while (true) {
            auto [x, z] = s.next();

            int worldSeed;
            if (!findInChunk(regionSeed, x, z, &worldSeed)) {
                continue;
            }

            pos->x = x * 16 + 4;
            pos->z = z * 16 + 4;
            return worldSeed;
        }
    }
}
