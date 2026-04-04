#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>


extern "C" {
    #include "cubiomes-viewer-bedrock/cubiomes/mt.h"
    #include "cubiomes-viewer-bedrock/cubiomes/biomes.h"
    #include "cubiomes-viewer-bedrock/cubiomes/finders.h"
}

const int mc = MC_1_2;
const int n = 1;//std::thread::hardware_concurrency();
std::mutex mtx;

namespace config {
    const int chunk = 20000;
    const int reportInterval = 1 << 25;
    const int minTorches = 20;
    const uint32_t minSeed = 0;
    const uint32_t maxSeed = UINT32_MAX;
};

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

#define SALT 10387312L

uint32_t getVillageRegionSeed(int worldSeed, int chunkX, int chunkZ) {
    int regX = chunkX < 0 ? chunkX - 39 : chunkX;
    int regZ = chunkZ < 0 ? chunkZ - 39 : chunkZ;

    return regX * REGION_A + regZ * REGION_B + worldSeed + SALT;
}

int getVillageWorldSeed(uint32_t regionSeed, int chunkX, int chunkZ) {
    int regX = chunkX < 0 ? chunkX - 39 : chunkX;
    int regZ = chunkZ < 0 ? chunkZ - 39 : chunkZ;

    int64_t C = regX * REGION_A + regZ * REGION_B + SALT;
    return regionSeed - (uint32_t)C;
}

std::atomic<uint32_t> nextSeed(config::minSeed);
auto startTime = std::chrono::steady_clock::now();

void printCsvHeader() {
    std::cout << "Seed,Torches,Location," << std::endl;
}

void printCsvRow(int seed, int torches, Pos location) {
    std::cout << seed << ",";
    std::cout << torches << ",";
    std::cout << "\"" << location.x << ", " << location.z << "\",";
    std::cout << std::endl;
}

void checkSeed(uint32_t seed, StructureConfig sc, Piece *houses) {
    int torchCount = 0;
    setSeed(seed);
    getPreVillagePiecesNoSet(houses, 100, 0, 0, &torchCount);
    if (torchCount < config::minTorches) return;

    Spiral s;
    // int found = 0;
    while (true) {
        auto [x, z] = s.next();

        setSeed(seed);
        if (!isVillageChunkNoSet(x, z)) continue;

        int worldSeed = getVillageWorldSeed(seed, x, z);
        Pos village = { x * 16 + 4, z * 16 + 4 };

        Generator g;
        setupGenerator(&g, mc, 0);
        applySeed(&g, DIM_OVERWORLD, (uint32_t)worldSeed);

        if (!isViableStructurePos(Village, &g, village.x, village.z, 0)) continue;

        mtx.lock();
        printCsvRow(worldSeed, torchCount, village);
        mtx.unlock();

        return;
    }
}

void searchDynamic() {
    StructureConfig sc;
    getStructureConfig(Village, mc, &sc);
    Piece houses[100];

    while (true) {
        uint32_t start = nextSeed.fetch_add(config::chunk);
        if (start > config::maxSeed) break;

        uint32_t end;
        if (start > config::maxSeed - config::chunk) {
            end = config::maxSeed;
        } else {
            end = start + config::chunk;
        }

        for (uint32_t seed = start; seed <= end; seed++) {
            if (seed % config::reportInterval == 0 && seed != config::minSeed) {
                uint32_t seedsProcessed = nextSeed.load() - config::minSeed;
                auto elapsed = std::chrono::steady_clock::now() - startTime;
                double seconds = std::chrono::duration<double>(elapsed).count();
                double rate = static_cast<double>(seedsProcessed) / seconds;
                mtx.lock();
                std::cerr << "Processed " << seedsProcessed << " seeds, " << rate << " seeds/sec" << std::endl;
                mtx.unlock();
            }

            checkSeed(seed, sc, houses);
        }
    }
}

int main() {
    printCsvHeader();


    StructureConfig sc;
    getStructureConfig(Village, mc, &sc);
    Piece houses[100];
    auto r = getVillageRegionSeed(1977692756, -220>>4, 804>>4);
    checkSeed(r, sc, houses);
    return 0;

    // std::cout << r << std::endl;
    // return 0;

    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < n; i++) {
        threads.emplace_back(searchDynamic);
    }
    for (auto &t : threads) t.join();

    return 0;
}
