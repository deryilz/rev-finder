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

    void processVillage(int regionSeed) override {
        Piece houses[100];

        int torchCount = 0;
        setSeed(regionSeed);
        getPreVillagePiecesNoSet(houses, 100, 0, 0, &torchCount);

        if (torchCount < min) return;

        Pos village;
        int worldSeed = rev::findClosestViable(regionSeed, &village);

        print(std::format("{},{},\"{},{}\"", worldSeed, torchCount, village.x, village.z));
    };
};
