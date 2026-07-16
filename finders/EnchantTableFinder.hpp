#include <format>

#include "Finder.hpp"

// TODO THIS IS FOR SOMETHING KHALOOODY WANTS

class EnchantTableFinder : public Finder {
    void printHeader() override {
        print("Seed");
    };

    void processVillage(int regionSeed) override {
        Piece houses[100];

        int torchCount = 0;
        setSeed(regionSeed);
        int len = getPreVillagePiecesNoSet(houses, 100, 0, 0, &torchCount);

        for (int i=0; i<len; i++) {
            if (houses[i].type == Blacksmith) {
                print(std::format("{}", regionSeed));
            }
        }
    }
};
