#include <format>
#include <algorithm>

#include "Finder.hpp"
#include "../utils/utils.hpp"

class LargeVillageFinder : public Finder {
public:
    double bestYet = 0.0;

    void printHeader() override {
        print("Seed,Size,Location");
    };

    void checkSeed(int regionSeed) override {
        Piece houses[100];

        int torchCount = 0;
        setSeed(regionSeed);
        int len = getPreVillagePiecesNoSet(houses, 100, 0, 0, &torchCount);

        int minX = houses[0].bb0.x;
        int maxX = houses[0].bb1.x;
        int minZ = houses[0].bb0.z;
        int maxZ = houses[0].bb1.z;

        for (int i=1;i<len;i++) {
            if (
                houses[i].type == VP_TORCH ||
                houses[i].type == VP_PATH ||
                houses[i].type == VP_FIELD1 ||
                houses[i].type == VP_FIELD2
            ) { continue; }
            minX = std::min(minX, houses[i].bb0.x);
            maxX = std::max(maxX, houses[i].bb1.x);
            minZ = std::min(minZ, houses[i].bb0.z);
            maxZ = std::max(maxZ, houses[i].bb1.z);
        }

        double size = std::max(maxZ - minZ, maxX - minX);

        if (size <= bestYet) return;
        bestYet = size;

        std::string lines = "";

        VillageIter v(regionSeed);
        for (int i=0; i<5; i++) {
            Pos village;
            int worldSeed = v.nextSpawn(&village);
            lines.append(std::format("({}) {},{},\"{},{}\"\n", regionSeed, worldSeed, size, village.x, village.z));
        }

        print(lines);
    }
};
