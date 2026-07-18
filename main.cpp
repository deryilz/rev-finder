#include "finders/HighShVillageFinder.hpp"

int main() {
    Finder* finder = new HighShVillageFinder(36, 49);
    finder->startSearch(INT32_MIN);

    return 0;
}
