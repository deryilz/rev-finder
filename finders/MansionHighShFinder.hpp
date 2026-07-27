#include <cmath>
#include <format>

#include "Finder.hpp"
#include "../utils/utils.hpp"
#include "../utils/gauss.hpp"

static uint32_t targets[] = {
    37333113, 59554629, 99674413, 135690917,
    147430197, 213151472, 257791329, 273514613,
    328361896, 343795143, 374874168, 401200881,
    403585984, 413695336, 438730053, 461412549,
    470269338, 639761397, 642870113, 660103073,
    739840719, 844507943, 880654251, 886804197,
    889355280, 897109831, 903125220, 922885111,
    928570448, 935644684, 962386929, 1001439820,
    1020452620, 1048229359, 1068341878, 1109650131,
    1160527633, 1162930020, 1167209268, 1177172004,
    1223686713, 1235173182, 1270379033, 1301672055,
    1311285322, 1330796976, 1365129300, 1394515379,
    1460927482, 1461655638, 1482694437, 1509300525,
    1518457214, 1525487214, 1558828389, 1592209739,
    1613387471, 1617281787, 1670756161, 1693283794,
    1708161006, 1727569366, 1730157349, 1784649745,
    1796403545, 1803835648, 1815508399, 1819446407,
    1836315345, 1851321387, 1869514882, 1931350606,
    1938142252, 1939184840, 1974594829, 1988781485,
    1995942318, 2043576986, 2058865618, 2067111155,
    2141085613, 2156633257, 2198460579, 2242471255,
    2253667144, 2260723908, 2345045245, 2407125674,
    2425033926, 2451930346, 2463758693, 2470961764,
    2471853506, 2486035358, 2494267334, 2521499299,
    2537499593, 2547953771, 2622634394, 2624527239,
    2690269654, 2694509759, 2697224738, 2715315102,
    2746320468, 2752111907, 2783345743, 2786626657,
    2810062732, 2838891414, 2842698246, 2851475621,
    2878175960, 2905167325, 2916237205, 2918406033,
    2952289789, 2971588887, 2972139569, 2984671649,
    3004338420, 3059730970, 3126626288, 3130877723,
    3139607200, 3145755799, 3150228515, 3236568688,
    3316723168, 3345818081, 3357924277, 3369382265,
    3380128365, 3409069794, 3449386450, 3503720374,
    3527808968, 3528033525, 3564097730, 3571982813,
    3576917795, 3584774985, 3603804589, 3630201793,
    3658176389, 3694592759, 3700160837, 3712112775,
    3716481319, 3737785819, 3752551599, 3774650239,
    3813919941, 3815376674, 3849019473, 3887981839,
    3898581978, 3915215437, 3921203679, 3936473789,
    3954531583, 4016441406, 4018987072, 4044210756,
    4061076252, 4142507097, 4164917266, 4180682555,
    4232312206, 4249174359, 4259515654, 4274262272,
};

class MansionHighShFinder : public Finder {
    int chunkMax;
    StructureConfig mansionConfig;
    StructureConfig strongholdConfig;

public:
    MansionHighShFinder(int chunkMax = 6250) {
        this->chunkMax = chunkMax;

        getStructureConfig(Mansion, MC_1_16, &mansionConfig);
        getStructureConfig(Stronghold, MC_1_16, &strongholdConfig);
    }

    void printHeader() override {
        print("Seed,Mansion,Closest High Room,Dist,Max Height,Stronghold Seed");
    };

    void checkSeed(int seed) override {
        GaussInfo info (seed);
        for (uint32_t target : targets) {
            GaussChunkIter iter (info, target, chunkMax);
            Pos pos;
            while (iter.nextMatch(&pos)) {
                handleMatch(seed, target, pos.x >> 4, pos.z >> 4);
            }
        }
    }

    void handleMatch(int seed, uint32_t strongholdSeed, int chunkX, int chunkZ) {
        Pos sh;
        Pos reg = chunkToRegion(chunkX, chunkZ, strongholdConfig.regionSize);
        if (!getStaticStronghold(strongholdConfig, seed, reg.x, reg.z, &sh)) {
            return;
        }

        if ((sh.x >> 4) != chunkX || (sh.z >> 4) != chunkZ) {
            return;
        }

        reg = chunkToRegion(chunkX, chunkZ, mansionConfig.regionSize);
        Pos mansion = getLargeStructurePos(mansionConfig, seed, reg.x, reg.z);
        if (coords::dist(sh, mansion) > 300) {
            return;
        }

        Generator g;
        setupGenerator(&g, MC_1_16, 0);
        applySeed(&g, DIM_OVERWORLD, (uint32_t)seed);

        if (!isViableStructurePos(Mansion, &g, mansion.x, mansion.z, 0)) {
            return;
        }

        Piece pieces[SH_PIECES_MAX];
        int len = getStrongholdPieces(pieces, SH_PIECES_MAX, MC_1_16, seed, chunkX, chunkZ, true);

        int maxHeight = 0;
        Piece *closestHighRoom = nullptr;
        double bestDist = INFINITY;

        for (Piece &piece : pieces) {
            int y = piece.bb1.y;
            if (y > maxHeight) {
                maxHeight = y;
            }

            if (y >= 67) {
                Pos pos { piece.pos.x, piece.pos.z };
                double dist = coords::dist(mansion, pos);
                if (dist < bestDist) {
                    bestDist = dist;
                    closestHighRoom = &piece;
                }
            }
        }

        auto showPiece = [](Piece p) {
            return std::format("/tp {} {} {}", p.pos.x, p.bb1.y, p.pos.z);
        };
        auto showPos = [](Pos p) {
            return std::format("/tp {} 100 {}", p.x, p.z);
        };

        print(std::format(
            "{},{},{},{},{}",
            seed, showPos(mansion), showPiece(*closestHighRoom),
            bestDist, maxHeight, strongholdSeed
        ));
    }

};
