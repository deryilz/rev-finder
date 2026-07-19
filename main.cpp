#include "modifiers/HighShVillageModifier.hpp"
#include "finders/HighShVillageFinder.hpp"

int main() {
    HighShVillageModifier mod {};
    if (mod.openFile("high-sh-villages-3200.txt")) {
        mod.startModifying();
    }

    // HighShVillageFinder finder(100, 144);
    // finder.startSearch(-1107143648);

    return 0;
}
