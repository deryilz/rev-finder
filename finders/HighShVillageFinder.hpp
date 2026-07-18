#include <format>
#include <vector>

#include "Finder.hpp"
#include "../utils/utils.hpp"

class HighShVillageFinder : public Finder {
    // you're gonna hate me for this
    inline bool isGoodShSeed(uint32_t regionSeed) {
        switch (regionSeed) {
            case 37333113:
            case 59554629:
            case 99674413:
            case 135690917:
            case 147430197:
            case 213151472:
            case 257791329:
            case 273514613:
            case 328361896:
            case 343795143:
            case 374874168:
            case 401200881:
            case 403585984:
            case 413695336:
            case 438730053:
            case 461412549:
            case 470269338:
            case 639761397:
            case 642870113:
            case 660103073:
            case 739840719:
            case 844507943:
            case 880654251:
            case 886804197:
            case 889355280:
            case 897109831:
            case 903125220:
            case 922885111:
            case 928570448:
            case 935644684:
            case 962386929:
            case 1001439820:
            case 1020452620:
            case 1048229359:
            case 1068341878:
            case 1109650131:
            case 1160527633:
            case 1162930020:
            case 1167209268:
            case 1177172004:
            case 1223686713:
            case 1235173182:
            case 1270379033:
            case 1301672055:
            case 1311285322:
            case 1330796976:
            case 1365129300:
            case 1394515379:
            case 1460927482:
            case 1461655638:
            case 1482694437:
            case 1509300525:
            case 1518457214:
            case 1525487214:
            case 1558828389:
            case 1592209739:
            case 1613387471:
            case 1617281787:
            case 1670756161:
            case 1693283794:
            case 1708161006:
            case 1727569366:
            case 1730157349:
            case 1784649745:
            case 1796403545:
            case 1803835648:
            case 1815508399:
            case 1819446407:
            case 1836315345:
            case 1851321387:
            case 1869514882:
            case 1931350606:
            case 1938142252:
            case 1939184840:
            case 1974594829:
            case 1988781485:
            case 1995942318:
            case 2043576986:
            case 2058865618:
            case 2067111155:
            case 2141085613:
            case 2156633257:
            case 2198460579:
            case 2242471255:
            case 2253667144:
            case 2260723908:
            case 2345045245:
            case 2407125674:
            case 2425033926:
            case 2451930346:
            case 2463758693:
            case 2470961764:
            case 2471853506:
            case 2486035358:
            case 2494267334:
            case 2521499299:
            case 2537499593:
            case 2547953771:
            case 2622634394:
            case 2624527239:
            case 2690269654:
            case 2694509759:
            case 2697224738:
            case 2715315102:
            case 2746320468:
            case 2752111907:
            case 2783345743:
            case 2786626657:
            case 2810062732:
            case 2838891414:
            case 2842698246:
            case 2851475621:
            case 2878175960:
            case 2905167325:
            case 2916237205:
            case 2918406033:
            case 2952289789:
            case 2971588887:
            case 2972139569:
            case 2984671649:
            case 3004338420:
            case 3059730970:
            case 3126626288:
            case 3130877723:
            case 3139607200:
            case 3145755799:
            case 3150228515:
            case 3236568688:
            case 3316723168:
            case 3345818081:
            case 3357924277:
            case 3369382265:
            case 3380128365:
            case 3409069794:
            case 3449386450:
            case 3503720374:
            case 3527808968:
            case 3528033525:
            case 3564097730:
            case 3571982813:
            case 3576917795:
            case 3584774985:
            case 3603804589:
            case 3630201793:
            case 3658176389:
            case 3694592759:
            case 3700160837:
            case 3712112775:
            case 3716481319:
            case 3737785819:
            case 3752551599:
            case 3774650239:
            case 3813919941:
            case 3815376674:
            case 3849019473:
            case 3887981839:
            case 3898581978:
            case 3915215437:
            case 3921203679:
            case 3936473789:
            case 3954531583:
            case 4016441406:
            case 4018987072:
            case 4044210756:
            case 4061076252:
            case 4142507097:
            case 4164917266:
            case 4180682555:
            case 4232312206:
            case 4249174359:
            case 4259515654:
            case 4274262272:
                return true;
            default:
                return false;
        }
    }

public:
    int start;
    int stop;

    HighShVillageFinder(int start = 4, int stop = 36) {
        this->start = start;
        this->stop = stop;
    }

    void printHeader() override {
        print("Seed,Stronghold Seed,Location");
    };

    void checkSeed(int regionSeed) override {
        VillageIter v(regionSeed);
        v.skip(start);

        for(int i=0; i<stop-start; i++) {
            Pos village;
            int worldSeed = v.nextCandidate(&village);

            Pos chunk = { village.x >> 4, village.z >> 4 };
            uint32_t shSeed = rev::getStrongholdRegionSeed(worldSeed, chunk.x, chunk.z);

            if (!isGoodShSeed(shSeed)) continue;

            Generator g;
            setupGenerator(&g, MC_1_2, 0);
            applySeed(&g, DIM_OVERWORLD, (uint32_t)worldSeed);

            if (!isViableStructurePos(Village, &g, village.x, village.z, 0)) {
                continue;
            }

            bool wtf = false;
            StrongholdIter sh {};
            while (nextVillageStronghold(&sh, &g) > 0) {
                if (sh.pos.x >> 4 == village.x >> 4 && sh.pos.z >> 4 == village.z >> 4) {
                    wtf = true;
                }
            }
            if (!wtf) return;

            print(std::format(
                "{},{},\"{},{}\"",
                worldSeed, shSeed, village.x, village.z
            ));
        }
    }
};
