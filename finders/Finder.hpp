#include <iostream>
#include <cstdint>
#include <mutex>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <format>
#include <string>

extern "C" {
    #include "../cubiomes-viewer-bedrock/cubiomes/finders.h"
}

class Finder {
public:
    int chunk = 20000;
    int reportInterval = 1 << 25;
    int numThreads;
    std::mutex mtx;
    std::atomic<int64_t> nextSeed;
    std::chrono::steady_clock::time_point startTime;

    StructureConfig sc;

    Finder() {
        getStructureConfig(Village, MC_1_2, &sc);
        numThreads = std::thread::hardware_concurrency();
    }

    virtual void printHeader() = 0;
    virtual void processVillage(int regionSeed) = 0;

    void print(std::string str, bool err = false) {
        mtx.lock();
        (err ? std::cerr : std::cout) << str << std::endl;
        mtx.unlock();
    }

    // assumes it's linear
    void printRate(int minSeed) {
        uint64_t seedsProcessed = abs(nextSeed.load() - minSeed);
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        double seconds = std::chrono::duration<double>(elapsed).count();
        double rate = static_cast<double>(seedsProcessed) / seconds;
        print(std::format("Processed {} seeds, {} seeds/sec", seedsProcessed, rate), true);
    }

    void startSearch(int minSeed = INT32_MIN, int maxSeed = INT32_MAX) {
        nextSeed = minSeed;
        printHeader();

        startTime = std::chrono::steady_clock::now();

        std::vector<std::thread> threads;
        for (unsigned int i = 0; i < numThreads; i++) {
            threads.emplace_back([&](){
                while (true) {
                    int64_t start = nextSeed.fetch_add(chunk);
                    if (start > maxSeed) break;

                    int64_t end;
                    if (start > maxSeed - chunk) {
                        end = maxSeed;
                    } else {
                        end = start + chunk;
                    }

                    for (int64_t seed = start; seed <= end; seed++) {
                        if (seed % reportInterval == 0 && seed != minSeed) {
                            printRate(minSeed);
                        }

                        processVillage(seed);
                    }
                }
            });
        }

        for (auto &t : threads) t.join();
    }
};
