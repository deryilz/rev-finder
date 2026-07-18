#include "finders/HighShVillageFinder.hpp"

int main() {
    Finder* finder = new HighShVillageFinder();
    finder->startSearch(INT32_MIN + 2986380000);

    return 0;
}
