#include "finders/TorchFinder.hpp"

int main() {
    Finder* finder = new TorchFinder(21);
    finder->startSearch(0);

    return 0;
}
