#pragma once

#include <format>

#include "Finder.hpp"
#include "../utils/utils.hpp"

class TorchFinder : public Finder {
public:
    int min;

    TorchFinder(int minTorches) {
        min = minTorches;
    }

    void printHeader() override {
        mtx.lock();
        std::cout << "Seed,Torches,Location" << std::endl;
        mtx.unlock();
    };

    void processVillage(int regionSeed) override {
        Piece houses[100];

        int torchCount = 0;
        setSeed(regionSeed);
        getPreVillagePiecesNoSet(houses, 100, 0, 0, &torchCount);

        if (torchCount < min) return;

        Spiral s;
        // int found = 0;
        while (true) {
            auto [x, z] = s.next();

            setSeed(regionSeed);
            if (!isVillageChunkNoSet(x, z)) continue;

            int worldSeed = rev::getVillageWorldSeed(regionSeed, x, z);
            Pos village = { x * 16 + 4, z * 16 + 4 };

            Generator g;
            setupGenerator(&g, mc, 0);
            applySeed(&g, DIM_OVERWORLD, (uint32_t)worldSeed);

            if (!isViableStructurePos(Village, &g, village.x, village.z, 0)) continue;

            print(std::format("{},{},\"{},{}\"", worldSeed, torchCount, village.x, village.z));

            return;
        }
    };
};
