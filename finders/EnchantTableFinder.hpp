#include <cmath>
#include <format>

#include "Finder.hpp"
#include "../utils/utils.hpp"

class EnchantTableFinder : public Finder {
public:
    double maxDist;

    EnchantTableFinder(double maxDist) {
        this->maxDist = maxDist;
    }

    void printHeader() override {
        print("Seed,Dist,Window,Door,Village Chunk");
    };

    void checkSeed(int regionSeed) override {
        Piece houses[100];

        int torchCount = 0;
        setSeed(regionSeed);
        // TODO: getHouseListNoSet?
        int len = getPreVillagePiecesNoSet(houses, 100, 0, 0, &torchCount);

        Pos spawnLocation = { 0, 4 };

        bool foundBlacksmith = false;
        Pos windowLocation;
        Pos badLocation;
        for (int i=0; i<len; i++) {
            if (houses[i].type == VP_HOUSE2) {
                foundBlacksmith = true;
                windowLocation = coords::pieceOffset(houses[i], 4, 10);
                badLocation = coords::pieceOffset(houses[i], 4, 5);
            }
        }
        if (!foundBlacksmith) return;

        bool foundLibrary = false;
        double bestDist = INFINITY;
        Pos doorLocation;
        for (int i=0; i<len; i++) {
            if (houses[i].type == VP_HOUSE1) {
                Pos newLocation = coords::pieceOffset(houses[i], 3, -3);
                double angle = coords::angle(windowLocation, badLocation, newLocation);
                if (angle > M_PI * 0.5) {
                    continue;
                }

                foundLibrary = true;
                double newDist = coords::dist(newLocation, windowLocation);
                if (newDist < bestDist) {
                    bestDist = newDist;
                    doorLocation = newLocation;
                }
            }
        }
        if (!foundLibrary) return;

        // we assume spawn to be (0, 4), so window should be as close as possible
        if (windowLocation.x > 16 || windowLocation.z > 16) return;
        int chunkX = (int)round((double)(spawnLocation.x - windowLocation.x) / 16);
        int chunkZ = (int)round((double)(spawnLocation.z - windowLocation.z) / 16);

        // update positions
        for (Pos *p : {&windowLocation, &badLocation, &doorLocation}) {
            p->x += chunkX * 16;
            p->z += chunkZ * 16;
        }

        double totalDist = coords::dist(spawnLocation, windowLocation);
        totalDist += bestDist;
        totalDist = round(totalDist * 100) / 100;
        if (totalDist > maxDist) {
            return;
        }

        int worldSeed;
        if (!rev::doesVillageSpawn(regionSeed, chunkX, chunkZ, &worldSeed)) {
            return;
        }

        Generator g;
        setupGenerator(&g, MC_1_2, 0);
        applySeed(&g, DIM_OVERWORLD, (uint32_t)worldSeed);
        Pos p = getSpawn(&g);
        if (p.x > 4 || p.z > 4) {
            return;
        }

        print(std::format(
            "{},{},\"{},{}\",\"{},{}\",\"{},{}\"",
            worldSeed, totalDist,
            windowLocation.x, windowLocation.z,
            // badLocation.x, badLocation.z,
            doorLocation.x, doorLocation.z,
            chunkX, chunkZ
        ));
    }
};
