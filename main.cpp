#include "finders/EnchantTableFinder.hpp"

int main() {
    Finder* finder = new EnchantTableFinder(18);
    finder->startSearch();

    return 0;
}
