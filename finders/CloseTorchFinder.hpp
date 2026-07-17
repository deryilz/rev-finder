#include <format>
#include <vector>

#include "Finder.hpp"
#include "../utils/utils.hpp"

class CloseTorchFinder : public Finder {
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

        if (torchCount < 10) return;

        std::vector<Pos> torches {};
        for (int i=1;i<len;i++) {
            if (houses[i].type == VP_TORCH) {
                Pos location = { houses[i].pos.x, houses[i].pos.z };
                torches.push_back(location);
            }
        }


        double sum = 0.0;
        int count = 0;

        for (int i = 0; i < torches.size(); i++) {
            for (int j = i + 1; j < torches.size(); j++) {
                sum += coords::dist(torches[i], torches[j]);
                count++;
            }
        }

        double score = 1000 + 2 * torchCount - sum / count;
        if (score <= bestYet) return;
        bestYet = score;

        std::string lines = "";

        VillageIter v(regionSeed);
        for (int i=0; i<5; i++) {
            Pos village;
            int worldSeed = v.nextSpawn(&village);
            lines.append(std::format(
                "({}) {},{},{},\"{},{}\"\n",
                regionSeed, torchCount, worldSeed, score, village.x, village.z
            ));
        }

        print(lines);
    }
};
