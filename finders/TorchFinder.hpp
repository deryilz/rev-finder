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
        print("Seed,Torches,Location");
    };

    void checkSeed(int regionSeed) override {
        Piece houses[100];

        int torchCount = 0;
        setSeed(regionSeed);
        getPreVillagePieces(houses, 100, 0, 0, 0, &torchCount, false);

        if (torchCount < min) return;

        Pos village;
        VillageIter v(regionSeed);
        int worldSeed = v.nextSpawn(&village);

        print(std::format("{},{},\"{},{}\"", worldSeed, torchCount, village.x, village.z));
    };
};
