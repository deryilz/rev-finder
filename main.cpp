extern "C" {
    #include "cubiomes-viewer-bedrock/cubiomes/mt.h"
    #include "cubiomes-viewer-bedrock/cubiomes/biomes.h"
    #include "cubiomes-viewer-bedrock/cubiomes/finders.h"
}

#include "finders/TorchFinder.hpp"

int main() {
    Finder* finder = new TorchFinder(21);
    finder->startSearch(0);

    return 0;
}
