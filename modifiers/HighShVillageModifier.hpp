#include <format>

#include "Modifier.hpp"

class HighShVillageModifier : public Modifier {
public:
    void printHeader() override {
        print("Seed,Village,Distance from spawn,Max height,Starter,Portal room,Highest other room,Distance from starter,Area over water level,Stronghold seed");
    }

    void checkParts(std::vector<std::string> parts) override {
        int seed = str::num(parts[0]);
        int strongholdSeed = str::num(parts[1]);
        int x = str::num(parts[2]);
        int z = str::num(parts[3]);

        Piece pieces[SH_PIECES_MAX];
        int len = getStrongholdPieces(pieces, SH_PIECES_MAX, MC_1_2, seed, x>>4, z>>4, true);

        int maxHeight = 0;
        Piece *starter = &pieces[0];
        Piece *portalRoom = nullptr;
        Piece *highestOther = starter;

        for(Piece &piece : pieces) {
            int y = piece.bb1.y;
            if (y > maxHeight) {
                maxHeight = y;
            }

            if (piece.type == SH_PORTAL_ROOM) {
                portalRoom = &piece;
            } else if (y > highestOther->bb1.y) {
                highestOther = &piece;
            }
        }

        int minDistOfHighest = INT32_MAX; // from starter
        int areaOverWater = 0;

        // second pass yay
        for(Piece &piece : pieces) {
            int y = piece.bb1.y;
            if (y > 63) {
                int dx = piece.bb1.x - piece.bb0.x;
                int dz = piece.bb1.z - piece.bb0.z;
                areaOverWater += dx*dz;
            }

            if (y == maxHeight) {
                int dist = coords::dist(starter->pos, piece.pos);
                if (dist < minDistOfHighest) {
                    minDistOfHighest = dist;
                }
            }
        }

        Generator g;
        setupGenerator(&g, MC_1_2, 0);
        applySeed(&g, DIM_OVERWORLD, (uint32_t)seed);
        Pos spawn = getSpawn(&g);
        Pos village = {x, z};
        int villageDist = coords::dist(spawn, village);

        auto showCoord = [](Piece p) {
            return std::format("/tp {} {} {}", p.pos.x, p.bb1.y, p.pos.z);
        };
        auto showPos = [](Pos p) {
            return std::format("/tp {} 100 {}", p.x, p.z);
        };

        print(std::format(
            "{},{},{},{},{},{},{},{},{},{}",
            seed, showPos(village), villageDist, maxHeight,
            showCoord(*starter), portalRoom ? showCoord(*portalRoom) : "",
            showCoord(*highestOther), minDistOfHighest, areaOverWater, strongholdSeed
        ));
    }
};
