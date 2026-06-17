#include "../../include/constants/item.h"
#include "../../include/debug.h"
#include "../../include/pokemon.h"
#include "../../include/save.h"
#include "../../include/script.h"
#include "../../include/types.h"

#ifdef MART_EXPANSION

struct MartItem {
    u16 item_id;
    u16 override_cost;
};

struct BadgeMartItems {
    u16 item_id;
    u8 required_badges;
};

// note: limited to 203 items (~34 pages)
const struct BadgeMartItems sBadgeMart[] = {
    { ITEM_POKE_BALL,      0 },
    { ITEM_GREAT_BALL,     1 },
    { ITEM_ULTRA_BALL,     3 },
    { ITEM_POTION,         0 },
    { ITEM_SUPER_POTION,   1 },
    { ITEM_HYPER_POTION,   3 },
    { ITEM_MAX_POTION,     5 },
    { ITEM_FULL_RESTORE,   4 },
    { ITEM_REVIVE,         2 },
    { ITEM_ANTIDOTE,       1 },
    { ITEM_PARALYZE_HEAL,  1 },
    { ITEM_AWAKENING,      1 },
    { ITEM_BURN_HEAL,      1 },
    { ITEM_ICE_HEAL,       1 },
    { ITEM_FULL_HEAL,      2 },
    { ITEM_ESCAPE_ROPE,    1 },
    { ITEM_FIRE_GEM,       4 },
    { ITEM_WATER_GEM,      4 },
    { ITEM_ELECTRIC_GEM,   4 },
    { ITEM_GRASS_GEM,      4 },
    { ITEM_ICE_GEM,        4 },
    { ITEM_FIGHTING_GEM,   4 },
    { ITEM_POISON_GEM,     4 },
    { ITEM_GROUND_GEM,     4 },
    { ITEM_FLYING_GEM,     4 },
    { ITEM_PSYCHIC_GEM,    4 },
    { ITEM_BUG_GEM,        4 },
    { ITEM_ROCK_GEM,       4 },
    { ITEM_GHOST_GEM,      4 },
    { ITEM_DRAGON_GEM,     4 },
    { ITEM_DARK_GEM,       4 },
    { ITEM_STEEL_GEM,      4 },
    { ITEM_NORMAL_GEM,     4 },
    { ITEM_ABOMASITE,      4 },
    { ITEM_ABSOLITE,       5 },
    { ITEM_AERODACTYLITE,  4 },
    { ITEM_AGGRONITE,      5 },
    { ITEM_ALAKAZITE,      5 },
    { ITEM_ALTARIANITE,    5 },
    { ITEM_AMPHAROSITE,    5 },
    { ITEM_AUDINITE,       5 },
    { ITEM_BANETTITE,      4 },
    { ITEM_BEEDRILLITE,    4 },
    { ITEM_BLASTOISINITE,  4 },
    { ITEM_BLAZIKENITE,    4 },
    { ITEM_CAMERUPTITE,    4 },
    { ITEM_CHARIZARDITE_X, 4 },
    { ITEM_CHARIZARDITE_Y, 4 },
    { ITEM_DIANCITE,       7 },
    { ITEM_GALLADITE,      5 },
    { ITEM_GARCHOMPITE,    7 },
    { ITEM_GARDEVOIRITE,   5 },
    { ITEM_GENGARITE,      5 },
    { ITEM_GLALITITE,      4 },
    { ITEM_GYARADOSITE,    5 },
    { ITEM_HERACRONITE,    4 },
    { ITEM_HOUNDOOMINITE,  4 },
    { ITEM_KANGASKHANITE,  4 },
    { ITEM_LATIASITE,      8 },
    { ITEM_LATIOSITE,      8 },    
    { ITEM_LOPUNNITE,      4 },
    { ITEM_LUCARIONITE,    6 },
    { ITEM_MANECTITE,      4 },
    { ITEM_MAWILITE,       4 },
    { ITEM_MEDICHAMITE,    5 },
    { ITEM_METAGROSSITE,   7 },
    { ITEM_MEWTWONITE_X,   8 },
    { ITEM_MEWTWONITE_Y,   8 },
    { ITEM_PIDGEOTITE,     4 },
    { ITEM_PINSIRITE,      5 },
    { ITEM_SABLENITE,      4 },
    { ITEM_SALAMENCITE,    7 },
    { ITEM_SCEPTILITE,     4 },
    { ITEM_SCIZORITE,      5 },
    { ITEM_SHARPEDONITE,   4 },
    { ITEM_SLOWBRONITE,    5 },
    { ITEM_STEELIXITE,     4 },
    { ITEM_SWAMPERTITE,    4 },
    { ITEM_TYRANITARITE,   5 },
    { ITEM_VENUSAURITE,    4 },
};

void LONG_CALL InitMartUI(void *taskManager, FieldSystem *fieldSystem, const u16 *items, int kind, int buySell, int decoWhich, const struct MartItem *priceOverrides);

u16 sCherrygroveCityMart[] = {
    ITEM_NET_BALL, ITEM_HEAL_BALL, ITEM_ELIXIR, ITEM_ORAN_BERRY, 0xFFFF
};

u16 sVioletCityMart[] = {
    ITEM_NET_BALL, ITEM_LEVEL_BALL, ITEM_ELIXIR, ITEM_ORAN_BERRY, ITEM_ESCAPE_ROPE, 0xFFFF
};

u16 sAzaleaCityMart[] = {
    ITEM_NET_BALL, ITEM_LEVEL_BALL, ITEM_NEST_BALL, ITEM_ELIXIR, ITEM_ORAN_BERRY, ITEM_ESCAPE_ROPE, ITEM_CHESTO_BERRY, ITEM_PERSIM_BERRY, ITEM_CHERI_BERRY, ITEM_ASPEAR_BERRY, ITEM_RAWST_BERRY, 0xFFFF
};

u16 sGoldenrodDepartmentUpper2F[] = {
    ITEM_POTION, ITEM_SUPER_POTION, ITEM_HYPER_POTION, ITEM_MAX_POTION, ITEM_REVIVE,
    ITEM_MAX_REVIVE, ITEM_FULL_RESTORE, ITEM_ETHER, ITEM_MAX_ETHER, ITEM_BERRY_JUICE,
    ITEM_ESCAPE_ROPE, ITEM_TOXIC_ORB, ITEM_FLAME_ORB, ITEM_ELECTRIC_SEED, ITEM_GRASSY_SEED,
    ITEM_MISTY_SEED, ITEM_PSYCHIC_SEED, 0xFFFF
};

u16 sGoldenrodDepartmentLower2F[] = {
    ITEM_POKE_BALL, ITEM_GREAT_BALL, ITEM_ULTRA_BALL, ITEM_LEVEL_BALL, ITEM_NEST_BALL,
    ITEM_DIVE_BALL, ITEM_REPEAT_BALL, ITEM_TIMER_BALL, ITEM_LUXURY_BALL, ITEM_DUSK_BALL,
    ITEM_QUICK_BALL, ITEM_CHERISH_BALL, ITEM_FAST_BALL, ITEM_LEVEL_BALL, ITEM_LURE_BALL,
    ITEM_HEAVY_BALL, ITEM_LOVE_BALL, ITEM_FRIEND_BALL, ITEM_MOON_BALL, 0xFFFF
};

u16 sGoldenrodDepartment3F[] = {
    ITEM_FIRE_STONE, ITEM_WATER_STONE, ITEM_THUNDER_STONE, ITEM_LEAF_STONE, ITEM_ICE_STONE,
    ITEM_SHINY_STONE, ITEM_SUN_STONE, ITEM_MOON_STONE, ITEM_DUSK_STONE, ITEM_DAWN_STONE, 0xFFFF
};

u16 sGoldenrodDepartment4F[] = {
    ITEM_ORAN_BERRY, ITEM_SITRUS_BERRY, ITEM_FIGY_BERRY, ITEM_WIKI_BERRY, ITEM_MAGO_BERRY,
    ITEM_AGUAV_BERRY, ITEM_IAPAPA_BERRY, ITEM_OCCA_BERRY, ITEM_PASSHO_BERRY, ITEM_WACAN_BERRY,
    ITEM_RINDO_BERRY, ITEM_YACHE_BERRY, ITEM_CHOPLE_BERRY, ITEM_KEBIA_BERRY, ITEM_SHUCA_BERRY,
    ITEM_COBA_BERRY, ITEM_PAYAPA_BERRY, ITEM_TANGA_BERRY, ITEM_CHARTI_BERRY, ITEM_KASIB_BERRY,
    ITEM_HABAN_BERRY, ITEM_COLBUR_BERRY, ITEM_BABIRI_BERRY, ITEM_CHILAN_BERRY, 0xFFFF
};

u16 sGoldenrodDepartment5F[] = {
    ITEM_TR81, ITEM_TM091, ITEM_TM094, ITEM_TM188, ITEM_TM133,
    ITEM_TM101, ITEM_TM119, ITEM_TM112, ITEM_TM125, ITEM_TM126,
    ITEM_TM135, ITEM_TM115, 0xFFFF
};

u16 sGoldenrodHerbs[] = {
    ITEM_WHITE_HERB, ITEM_MENTAL_HERB, ITEM_POWER_HERB, ITEM_MIRROR_HERB, ITEM_HEAL_POWDER, ITEM_ENERGY_POWDER, ITEM_ENERGY_ROOT, ITEM_REVIVAL_HERB, 0xFFFF
};

u16 sEcruteakMart[] = {
    ITEM_NET_BALL, ITEM_LEVEL_BALL, ITEM_NEST_BALL, ITEM_DUSK_BALL, ITEM_ELIXIR, ITEM_ORAN_BERRY, ITEM_SITRUS_BERRY, ITEM_ESCAPE_ROPE, ITEM_CHESTO_BERRY, ITEM_PERSIM_BERRY, ITEM_CHERI_BERRY, ITEM_ASPEAR_BERRY, ITEM_RAWST_BERRY, 0xFFFF
};

u16 sOlivineMart[] = {
    ITEM_NET_BALL, ITEM_LEVEL_BALL, ITEM_NEST_BALL, ITEM_DUSK_BALL, ITEM_DIVE_BALL, ITEM_MAX_ELIXIR, ITEM_ORAN_BERRY, ITEM_ESCAPE_ROPE, ITEM_CHESTO_BERRY, ITEM_PERSIM_BERRY, ITEM_CHERI_BERRY, ITEM_ASPEAR_BERRY, ITEM_RAWST_BERRY, 0xFFFF
};

u16 sCianwoodPharmacy[] = {
    ITEM_POTION, ITEM_SUPER_POTION, ITEM_HYPER_POTION, ITEM_FULL_HEAL, ITEM_REVIVE, ITEM_WHITE_HERB, ITEM_MENTAL_HERB, ITEM_POWER_HERB, ITEM_MIRROR_HERB, 0xFFFF
};

u16 sBlackthornAndBattleFrontierMart[] = {
    ITEM_NET_BALL, ITEM_LEVEL_BALL, ITEM_NEST_BALL, ITEM_DUSK_BALL, ITEM_DIVE_BALL, ITEM_QUICK_BALL, ITEM_MAX_ELIXIR, ITEM_ORAN_BERRY, ITEM_SITRUS_BERRY, ITEM_ESCAPE_ROPE, ITEM_CHESTO_BERRY, ITEM_PERSIM_BERRY, ITEM_CHERI_BERRY, ITEM_ASPEAR_BERRY, ITEM_RAWST_BERRY, 0xFFFF
};

u16 sIndigoPlateau[] = {
    ITEM_ULTRA_BALL, ITEM_MAX_REPEL, ITEM_HYPER_POTION, ITEM_MAX_POTION, ITEM_FULL_RESTORE, ITEM_REVIVE, ITEM_FULL_HEAL, 0xFFFF
};

u16 sVermilionAndSafariMart[] = {
    ITEM_AIR_MAIL, ITEM_NEST_BALL, ITEM_DUSK_BALL, ITEM_QUICK_BALL, 0xFFFF
};

u16 sSaffronMart[] = {
    ITEM_AIR_MAIL, ITEM_DUSK_BALL, ITEM_QUICK_BALL, 0xFFFF
};

u16 sLavenderMart[] = {
    ITEM_AIR_MAIL, ITEM_DUSK_BALL, ITEM_QUICK_BALL, 0xFFFF
};

u16 sCeruleanMart[] = {
    ITEM_AIR_MAIL, ITEM_QUICK_BALL, 0xFFFF
};

u16 sCeladonDepartmentUpper2F[] = {
    ITEM_POTION, ITEM_SUPER_POTION, ITEM_HYPER_POTION, ITEM_MAX_POTION, ITEM_REVIVE, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL, ITEM_BURN_HEAL, ITEM_ICE_HEAL, ITEM_AWAKENING, ITEM_FULL_HEAL, 0xFFFF
};

u16 sCeladonDepartmentLower2F[] = {
    ITEM_POKE_BALL, ITEM_GREAT_BALL, ITEM_ULTRA_BALL, ITEM_ESCAPE_ROPE, ITEM_POKE_DOLL, ITEM_REPEL, ITEM_SUPER_REPEL, ITEM_MAX_REPEL, ITEM_GRASS_MAIL, ITEM_FLAME_MAIL, ITEM_BUBBLE_MAIL, ITEM_SPACE_MAIL, 0xFFFF
};

u16 sCeladonDepartment3F[] = {
    ITEM_TM021, ITEM_TM027, ITEM_TM087, ITEM_TM078, ITEM_TM012, ITEM_TM041, ITEM_TM020, ITEM_TM028, ITEM_TM076, ITEM_TM055, ITEM_TM072, ITEM_TM079, 0xFFFF
};

u16 sCeladonDepartment4F[] = {
    ITEM_AIR_MAIL, ITEM_TUNNEL_MAIL, ITEM_BLOOM_MAIL, 0xFFFF
};

u16 sCeladonDepartmentLeft5F[] = {
    ITEM_X_SPEED, ITEM_X_ATTACK, ITEM_X_DEFENSE, ITEM_GUARD_SPEC, ITEM_DIRE_HIT, ITEM_X_ACCURACY, ITEM_X_SP_ATK, ITEM_X_SP_DEF, 0xFFFF
};

u16 sCeladonDepartmentRight5F[] = {
    ITEM_PROTEIN, ITEM_IRON, ITEM_CALCIUM, ITEM_ZINC, ITEM_CARBOS, ITEM_HP_UP, 0xFFFF
};

u16 sFuschiaMart[] = {
    ITEM_STEEL_MAIL, ITEM_DUSK_BALL, ITEM_QUICK_BALL, 0xFFFF
};

u16 sPewterMart[] = {
    ITEM_STEEL_MAIL, ITEM_NEST_BALL, ITEM_QUICK_BALL, 0xFFFF
};

u16 sViridianMart[] = {
    ITEM_STEEL_MAIL, ITEM_NET_BALL, ITEM_HEAL_BALL, 0xFFFF
};

u16 sMtMoonSquare[] = {
    ITEM_POKE_DOLL, ITEM_FRESH_WATER, ITEM_SODA_POP, ITEM_LEMONADE, ITEM_REPEL, ITEM_HEART_MAIL, 0xFFFF
};

u16 sMahoganyPreRocketHideout[] = {
    ITEM_TINY_MUSHROOM, ITEM_POKE_BALL, ITEM_POTION, 0xFFFF
};

u16 sMahoganyPostRocketHideout[] = {
    ITEM_GREAT_BALL, ITEM_SUPER_POTION, ITEM_HYPER_POTION, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL, ITEM_SUPER_REPEL, ITEM_REVIVE, ITEM_AIR_MAIL, 0xFFFF
};

BOOL ScrCmd_MartBuy(SCRIPTCONTEXT *ctx)
{
    u16 unused UNUSED = ScriptGetVar(ctx);

    u16 items[NELEMS(sBadgeMart) + 1];
    u8 badgeCount = 0;
    u8 index = 0;
    u32 i;

    for (i = 0; i < 16; i++) {
        if (PlayerProfile_TestBadgeFlag(Sav2_PlayerData_GetProfileAddr(ctx->fsys->savedata), i) == TRUE) {
            badgeCount++;
        }
    }

    for (i = 0; i < NELEMS(sBadgeMart); i++) {
        if (badgeCount >= sBadgeMart[i].required_badges) {
            items[index] = sBadgeMart[i].item_id;
            index++;
        }
    }

    items[index] = 0xFFFF;
    InitMartUI(ctx->taskman, ctx->fsys, items, 0, 0, 0, 0); // this doesn't honor price overrides
    return TRUE;
}

#endif // MART_EXPANSION

#ifdef POKEATHLON_SHOP_EXPANSION

const struct MartItem sPokeathlonShop_Sunday[] = {
    { ITEM_RED_APRICORN, 200 },
    { ITEM_BLUE_APRICORN, 200 },
    { ITEM_BLACK_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_KINGS_ROCK, 3000 },
    { ITEM_HEART_SCALE, 1000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_Monday[] = {
    { ITEM_RED_APRICORN, 200 },
    { ITEM_BLUE_APRICORN, 200 },
    { ITEM_GREEN_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_MOON_STONE, 3000 },
    { ITEM_RARE_CANDY, 2000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_Tuesday[] = {
    { ITEM_YELLOW_APRICORN, 200 },
    { ITEM_PINK_APRICORN, 200 },
    { ITEM_WHITE_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_FIRE_STONE, 2500 },
    { ITEM_PP_UP, 1000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_Wednesday[] = {
    { ITEM_BLUE_APRICORN, 200 },
    { ITEM_PINK_APRICORN, 200 },
    { ITEM_BLACK_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_WATER_STONE, 2500 },
    { ITEM_HEART_SCALE, 1000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_Thursday[] = {
    { ITEM_YELLOW_APRICORN, 200 },
    { ITEM_PINK_APRICORN, 200 },
    { ITEM_WHITE_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_THUNDER_STONE, 2500 },
    { ITEM_PP_UP, 1000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_Friday[] = {
    { ITEM_RED_APRICORN, 200 },
    { ITEM_YELLOW_APRICORN, 200 },
    { ITEM_GREEN_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_METAL_COAT, 2500 },
    { ITEM_NUGGET, 500 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_Saturday[] = {
    { ITEM_GREEN_APRICORN, 200 },
    { ITEM_WHITE_APRICORN, 200 },
    { ITEM_BLACK_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_LEAF_STONE, 2500 },
    { ITEM_RARE_CANDY, 2000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_NatdexSunday[] = {
    { ITEM_RED_APRICORN, 200 },
    { ITEM_BLUE_APRICORN, 200 },
    { ITEM_BLACK_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_KINGS_ROCK, 3000 },
    { ITEM_HEART_SCALE, 1000 },
    { ITEM_FULL_RESTORE, 500 },
    { ITEM_NUGGET, 500 },
    { ITEM_SUN_STONE, 3000 },
    { ITEM_FIRE_STONE, 2500 },
    { ITEM_SHINY_STONE, 3000 },
    { ITEM_DAWN_STONE, 3000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_NatdexMonday[] = {
    { ITEM_RED_APRICORN, 200 },
    { ITEM_BLUE_APRICORN, 200 },
    { ITEM_GREEN_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_MOON_STONE, 3000 },
    { ITEM_RARE_CANDY, 2000 },
    { ITEM_FULL_RESTORE, 500 },
    { ITEM_KINGS_ROCK, 3000 },
    { ITEM_SUN_STONE, 3000 },
    { ITEM_WATER_STONE, 2500 },
    { ITEM_SHINY_STONE, 3000 },
    { ITEM_DUSK_STONE, 3000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_NatdexTuesday[] = {
    { ITEM_YELLOW_APRICORN, 200 },
    { ITEM_PINK_APRICORN, 200 },
    { ITEM_WHITE_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_FIRE_STONE, 2500 },
    { ITEM_PP_UP, 1000 },
    { ITEM_FULL_RESTORE, 500 },
    { ITEM_METAL_COAT, 2500 },
    { ITEM_WATER_STONE, 2500 },
    { ITEM_LEAF_STONE, 2500 },
    { ITEM_DUSK_STONE, 3000 },
    { ITEM_DAWN_STONE, 3000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_NatdexWednesday[] = {
    { ITEM_BLUE_APRICORN, 200 },
    { ITEM_PINK_APRICORN, 200 },
    { ITEM_BLACK_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_WATER_STONE, 2500 },
    { ITEM_HEART_SCALE, 1000 },
    { ITEM_FULL_RESTORE, 500 },
    { ITEM_DRAGON_SCALE, 2500 },
    { ITEM_THUNDER_STONE, 2500 },
    { ITEM_MOON_STONE, 3000 },
    { ITEM_SHINY_STONE, 3000 },
    { ITEM_DAWN_STONE, 3000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_NatdexThursday[] = {
    { ITEM_YELLOW_APRICORN, 200 },
    { ITEM_PINK_APRICORN, 200 },
    { ITEM_WHITE_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_THUNDER_STONE, 2500 },
    { ITEM_PP_UP, 1000 },
    { ITEM_FULL_RESTORE, 500 },
    { ITEM_KINGS_ROCK, 3000 },
    { ITEM_FIRE_STONE, 2500 },
    { ITEM_LEAF_STONE, 2500 },
    { ITEM_SHINY_STONE, 3000 },
    { ITEM_DUSK_STONE, 3000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_NatdexFriday[] = {
    { ITEM_RED_APRICORN, 200 },
    { ITEM_YELLOW_APRICORN, 200 },
    { ITEM_GREEN_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_METAL_COAT, 2500 },
    { ITEM_NUGGET, 500 },
    { ITEM_FULL_RESTORE, 500 },
    { ITEM_DRAGON_SCALE, 2500 },
    { ITEM_WATER_STONE, 2500 },
    { ITEM_SUN_STONE, 3000 },
    { ITEM_DUSK_STONE, 3000 },
    { ITEM_DAWN_STONE, 3000 },
    { 0xFFFF, 0 },
};

const struct MartItem sPokeathlonShop_NatdexSaturday[] = {
    { ITEM_GREEN_APRICORN, 200 },
    { ITEM_WHITE_APRICORN, 200 },
    { ITEM_BLACK_APRICORN, 200 },
    { ITEM_MOOMOO_MILK, 100 },
    { ITEM_LEAF_STONE, 2500 },
    { ITEM_RARE_CANDY, 2000 },
    { ITEM_FULL_RESTORE, 500 },
    { ITEM_METAL_COAT, 2500 },
    { ITEM_THUNDER_STONE, 2500 },
    { ITEM_SHINY_STONE, 3000 },
    { ITEM_DUSK_STONE, 3000 },
    { ITEM_DAWN_STONE, 3000 },
    { 0xFFFF, 0 },
};

#endif // POKEATHLON_SHOP_EXPANSION
