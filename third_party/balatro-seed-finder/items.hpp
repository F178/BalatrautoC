#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <iostream>


// High-performance enum-based item system for Balatro seed finder
// Replaces string-based lookups with fast integer operations

namespace Items {
    
    // Keep base cards as items
    
    std::vector<std::string> CARDS = {
        "C_2",
        "C_3",
        "C_4",
        "C_5",
        "C_6",
        "C_7",
        "C_8",
        "C_9",
        "C_A",
        "C_J",
        "C_K",
        "C_Q",
        "C_T",
        "D_2",
        "D_3",
        "D_4",
        "D_5",
        "D_6",
        "D_7",
        "D_8",
        "D_9",
        "D_A",
        "D_J",
        "D_K",
        "D_Q",
        "D_T",
        "H_2",
        "H_3",
        "H_4",
        "H_5",
        "H_6",
        "H_7",
        "H_8",
        "H_9",
        "H_A",
        "H_J",
        "H_K",
        "H_Q",
        "H_T",
        "S_2",
        "S_3",
        "S_4",
        "S_5",
        "S_6",
        "S_7",
        "S_8",
        "S_9",
        "S_A",
        "S_J",
        "S_K",
        "S_Q",
        "S_T"
    };

    // Core item type enumerations
    // Ordered by frequency of use for better cache performance

    
    enum class Joker : uint16_t {
        // Common Jokers (0-57)
        JOKER = 0,
        GREEDY_JOKER = 1,
        LUSTY_JOKER = 2,
        WRATHFUL_JOKER = 3,
        GLUTTONOUS_JOKER = 4,
        JOLLY_JOKER = 5,
        ZANY_JOKER = 6,
        MAD_JOKER = 7,
        CRAZY_JOKER = 8,
        DROLL_JOKER = 9,
        SLY_JOKER = 10,
        WILY_JOKER = 11,
        CLEVER_JOKER = 12,
        DEVIOUS_JOKER = 13,
        CRAFTY_JOKER = 14,
        HALF_JOKER = 15,
        CREDIT_CARD = 16,
        BANNER = 17,
        MYSTIC_SUMMIT = 18,
        EIGHT_BALL = 19,
        MISPRINT = 20,
        RAISED_FIST = 21,
        CHAOS_THE_CLOWN = 22,
        SCARY_FACE = 23,
        ABSTRACT_JOKER = 24,
        DELAYED_GRATIFICATION = 25,
        GROS_MICHEL = 26,
        EVEN_STEVEN = 27,
        ODD_TODD = 28,
        SCHOLAR = 29,
        BUSINESS_CARD = 30,
        SUPERNOVA = 31,
        RIDE_THE_BUS = 32,
        EGG = 33,
        RUNNER = 34,
        ICE_CREAM = 35,
        SPLASH = 36,
        BLUE_JOKER = 37,
        FACELESS_JOKER = 38,
        GREEN_JOKER = 39,
        SUPERPOSITION = 40,
        TO_DO_LIST = 41,
        CAVENDISH = 42,
        RED_CARD = 43,
        SQUARE_JOKER = 44,
        RIFF_RAFF = 45,
        PHOTOGRAPH = 46,
        RESERVED_PARKING = 47,
        MAIL_IN_REBATE = 48,
        HALLUCINATION = 49,
        FORTUNE_TELLER = 50,
        JUGGLER = 51,
        DRUNKARD = 52,
        GOLDEN_JOKER = 53,
        POPCORN = 54,
        WALKIE_TALKIE = 55,
        SMILEY_FACE = 56,
        GOLDEN_TICKET = 57,
        SWASHBUCKLER = 58,
        HANGING_CHAD = 59,
        SHOOT_THE_MOON = 60,
        
        // Uncommon Jokers (61-124)  
        JOKER_STENCIL = 61,
        FOUR_FINGERS = 62,
        MIME = 63,
        CEREMONIAL_DAGGER = 64,
        MARBLE_JOKER = 65,
        LOYALTY_CARD = 66,
        DUSK = 67,
        FIBONACCI = 68,
        STEEL_JOKER = 69,
        HACK = 70,
        PAREIDOLIA = 71,
        SPACE_JOKER = 72,
        BURGLAR = 73,
        BLACKBOARD = 74,
        SIXTH_SENSE = 75,
        CONSTELLATION = 76,
        HIKER = 77,
        CARD_SHARP = 78,
        MADNESS = 79,
        SEANCE = 80,
        VAMPIRE = 81,
        SHORTCUT = 82,
        HOLOGRAM = 83,
        CLOUD_9 = 84,
        ROCKET = 85,
        MIDAS_MASK = 86,
        LUCHADOR = 87,
        GIFT_CARD = 88,
        TURTLE_BEAN = 89,
        EROSION = 90,
        TO_THE_MOON = 91,
        STONE_JOKER = 92,
        LUCKY_CAT = 93,
        BULL = 94,
        DIET_COLA = 95,
        TRADING_CARD = 96,
        FLASH_CARD = 97,
        SPARE_TROUSERS = 98,
        RAMEN = 99,
        SELTZER = 100,
        CASTLE = 101,
        MR_BONES = 102,
        ACROBAT = 103,
        SOCK_AND_BUSKIN = 104,
        TROUBADOUR = 105,
        CERTIFICATE = 106,
        SMEARED_JOKER = 107,
        THROWBACK = 108,
        ROUGH_GEM = 109,
        BLOODSTONE = 110,
        ARROWHEAD = 111,
        ONYX_AGATE = 112,
        GLASS_JOKER = 113,
        SHOWMAN = 114,
        FLOWER_POT = 115,
        MERRY_ANDY = 116,
        OOPS_ALL_6S = 117,
        THE_IDOL = 118,
        SEEING_DOUBLE = 119,
        MATADOR = 120,
        SATELLITE = 121,
        CARTOMANCER = 122,
        ASTRONOMER = 123,
        BOOTSTRAPS = 124,
        
        // Rare Jokers (125-144)
        DNA = 125,
        VAGABOND = 126,
        BARON = 127,
        OBELISK = 128,
        BASEBALL_CARD = 129,
        ANCIENT_JOKER = 130,
        CAMPFIRE = 131,
        BLUEPRINT = 132,
        WEE_JOKER = 133,
        HIT_THE_ROAD = 134,
        THE_DUO = 135,
        THE_TRIO = 136,
        THE_FAMILY = 137,
        THE_ORDER = 138,
        THE_TRIBE = 139,
        STUNTMAN = 140,
        INVISIBLE_JOKER = 141,
        BRAINSTORM = 142,
        DRIVERS_LICENSE = 143,
        BURNT_JOKER = 144,
        
        // Legendary Jokers (145-149)
        CANIO = 145,
        TRIBOULET = 146,
        YORICK = 147,
        CHICOT = 148,
        PERKEO = 149,
        
        // Special markers for range checking
        COMMON_START = JOKER,
        COMMON_END = SHOOT_THE_MOON,
        UNCOMMON_START = JOKER_STENCIL,
        UNCOMMON_END = BOOTSTRAPS,
        RARE_START = DNA,
        RARE_END = BURNT_JOKER,
        LEGENDARY_START = CANIO,
        LEGENDARY_END = PERKEO,
        
        COUNT = 150,
        INVALID = 65535
    };
    
    enum class Tarot : uint8_t {
        THE_FOOL = 0,
        THE_MAGICIAN = 1,
        THE_HIGH_PRIESTESS = 2,
        THE_EMPRESS = 3,
        THE_EMPEROR = 4,
        THE_HIEROPHANT = 5,
        THE_LOVERS = 6,
        THE_CHARIOT = 7,
        JUSTICE = 8,
        THE_HERMIT = 9,
        THE_WHEEL_OF_FORTUNE = 10,
        STRENGTH = 11,
        THE_HANGED_MAN = 12,
        DEATH = 13,
        TEMPERANCE = 14,
        THE_DEVIL = 15,
        THE_TOWER = 16,
        THE_STAR = 17,
        THE_MOON = 18,
        THE_SUN = 19,
        JUDGEMENT = 20,
        THE_WORLD = 21,
        SPECIAL_THE_SOUL = 22,
        
        COUNT = 23,

        INVALID = 255
    };

    enum class PlayedHand : uint8_t {
        FIVE_OF_A_KIND = 0,
        FLUSH_HOUSE = 1,
        FLUSH_FIVE = 2,

        COUNT = 3,

        INVALID = 255
    };
    
    enum class Planet : uint8_t {
        MERCURY = 0,
        VENUS = 1,
        EARTH = 2,
        MARS = 3,
        JUPITER = 4,
        SATURN = 5,
        URANUS = 6,
        NEPTUNE = 7,
        PLUTO = 8,
        PLANET_X = 9,
        CERES = 10,
        ERIS = 11,
        SPECIAL_BLACK_HOLE = 12,
        COUNT = 13,

        INVALID = 255
    };
    
    enum class Spectral : uint8_t {
        FAMILIAR = 0,
        GRIM = 1,
        INCANTATION = 2,
        TALISMAN = 3,
        AURA = 4,
        WRAITH = 5,
        SIGIL = 6,
        OUIJA = 7,
        ECTOPLASM = 8,
        IMMOLATE = 9,
        ANKH = 10,
        DEJA_VU = 11,
        HEX = 12,
        TRANCE = 13,
        MEDIUM = 14,
        CRYPTID = 15,
        SPECTRAL_THE_SOUL = 16,
        SPECTRAL_BLACK_HOLE = 17,
        COUNT = 18,

        INVALID = 255
    };
    
    enum class Voucher : uint8_t {
        OVERSTOCK = 0,
        OVERSTOCK_PLUS = 1,
        CLEARANCE_SALE = 2,
        LIQUIDATION = 3,
        HONE = 4,
        GLOW_UP = 5,
        REROLL_SURPLUS = 6,
        REROLL_GLUT = 7,
        CRYSTAL_BALL = 8,
        OMEN_GLOBE = 9,
        TELESCOPE = 10,
        OBSERVATORY = 11,
        GRABBER = 12,
        NACHO_TONG = 13,
        WASTEFUL = 14,
        RECYCLOMANCY = 15,
        TAROT_MERCHANT = 16,
        TAROT_TYCOON = 17,
        PLANET_MERCHANT = 18,
        PLANET_TYCOON = 19,
        SEED_MONEY = 20,
        MONEY_TREE = 21,
        BLANK = 22,
        ANTIMATTER = 23,
        MAGIC_TRICK = 24,
        ILLUSION = 25,
        HIEROGLYPH = 26,
        PETROGLYPH = 27,
        DIRECTORS_CUT = 28,
        RETCON = 29,
        PAINT_BRUSH = 30,
        PALETTE = 31,
        
        COUNT = 32,
        INVALID = 255
    };
    
    enum class Tag : uint8_t {
        UNCOMMON_TAG = 0,
        RARE_TAG = 1,
        NEGATIVE_TAG = 2,
        FOIL_TAG = 3,
        HOLOGRAPHIC_TAG = 4,
        POLYCHROME_TAG = 5,
        INVESTMENT_TAG = 6,
        VOUCHER_TAG = 7,
        BOSS_TAG = 8,
        STANDARD_TAG = 9,
        CHARM_TAG = 10,
        METEOR_TAG = 11,
        BUFFOON_TAG = 12,
        HANDY_TAG = 13,
        GARBAGE_TAG = 14,
        ETHEREAL_TAG = 15,
        COUPON_TAG = 16,
        DOUBLE_TAG = 17,
        JUGGLE_TAG = 18,
        D6_TAG = 19,
        TOPUP_TAG = 20,
        SPEED_TAG = 21,
        ORBITAL_TAG = 22,
        ECONOMY_TAG = 23,
        
        COUNT = 24,
        INVALID = 255
    };
    
    enum class Boss : uint8_t {
        THE_ARM = 0,
        THE_CLUB = 1,
        THE_EYE = 2,
        AMBER_ACORN = 3,
        CERULEAN_BELL = 4,
        CRIMSON_HEART = 5,
        VERDANT_LEAF = 6,
        VIOLET_VESSEL = 7,
        THE_FISH = 8,
        THE_FLINT = 9,
        THE_GOAD = 10,
        THE_HEAD = 11,
        THE_HOOK = 12,
        THE_HOUSE = 13,
        THE_MANACLE = 14,
        THE_MARK = 15,
        THE_MOUTH = 16,
        THE_NEEDLE = 17,
        THE_OX = 18,
        THE_PILLAR = 19,
        THE_PLANT = 20,
        THE_PSYCHIC = 21,
        THE_SERPENT = 22,
        THE_TOOTH = 23,
        THE_WALL = 24,
        THE_WATER = 25,
        THE_WHEEL = 26,
        THE_WINDOW = 27,
        
        COUNT = 28,
        INVALID = 255
    };
    
    enum class Enhancement : uint8_t {
        NO_ENHANCEMENT = 0,
        BONUS = 1,
        MULT = 2,
        WILD = 3,
        GLASS = 4,
        STEEL = 5,
        STONE = 6,
        GOLD = 7,
        LUCKY = 8,
        
        COUNT = 9,
        INVALID = 255
    };
    
    enum class Edition : uint8_t {
        NO_EDITION = 0,
        FOIL = 1,
        HOLOGRAPHIC = 2,
        POLYCHROME = 3,
        NEGATIVE = 4,
        
        COUNT = 5,
        INVALID = 255
    };
    
    enum class Seal : uint8_t {
        NO_SEAL = 0,
        RED_SEAL = 1,
        BLUE_SEAL = 2,
        GOLD_SEAL = 3,
        PURPLE_SEAL = 4,
        
        COUNT = 5,
        INVALID = 255
    };
    
    enum class Pack : uint8_t {
        ARCANA_PACK = 0,
        JUMBO_ARCANA_PACK = 1,
        MEGA_ARCANA_PACK = 2,
        CELESTIAL_PACK = 3,
        JUMBO_CELESTIAL_PACK = 4,
        MEGA_CELESTIAL_PACK = 5,
        STANDARD_PACK = 6,
        JUMBO_STANDARD_PACK = 7,
        MEGA_STANDARD_PACK = 8,
        BUFFOON_PACK = 9,
        JUMBO_BUFFOON_PACK = 10,
        MEGA_BUFFOON_PACK = 11,
        SPECTRAL_PACK = 12,
        JUMBO_SPECTRAL_PACK = 13,
        MEGA_SPECTRAL_PACK = 14,
        
        COUNT = 15,
        INVALID = 255
    };
    
    // Forward declared structs that use enum types
    struct MixedArcanaPack {
        std::vector<Tarot> tarots;
        std::vector<Spectral> spectrals;
        std::vector<bool> isSpectral; // Track which slots are spectral vs tarot
    };

    struct NextPackData {
        int size;
        int choices;
        Pack type;
    };

    NextPackData convertPackData(Pack p) {
        uint8_t typeSize = static_cast<uint8_t>(p) % 3;
        Pack type = static_cast<Pack>((static_cast<uint8_t>(p) / 3) * 3);

        if(typeSize == 2) { // Mega packs
            return NextPackData{
                (type == Pack::BUFFOON_PACK || type == Pack::SPECTRAL_PACK) ? 4 : 5, // size
                2,
                type,
            };
        } else if (typeSize == 1) { // Jumbo Packs
            return NextPackData{
                (type == Pack::BUFFOON_PACK || type == Pack::SPECTRAL_PACK) ? 4 : 5, // size
                1,
                type,
            };
        } else {
            return NextPackData{
                (type == Pack::BUFFOON_PACK || type == Pack::SPECTRAL_PACK) ? 2 : 3, // size
                1,
                type,
            };
        }
    }
    
    // Grouped arrays for efficient cache access and rarity-based selection
    constexpr std::array<Joker, 61> COMMON_JOKERS = {
        Joker::JOKER,
        Joker::GREEDY_JOKER,
        Joker::LUSTY_JOKER,
        Joker::WRATHFUL_JOKER,
        Joker::GLUTTONOUS_JOKER,
        Joker::JOLLY_JOKER,
        Joker::ZANY_JOKER,
        Joker::MAD_JOKER,
        Joker::CRAZY_JOKER,
        Joker::DROLL_JOKER,
        Joker::SLY_JOKER,
        Joker::WILY_JOKER,
        Joker::CLEVER_JOKER,
        Joker::DEVIOUS_JOKER,
        Joker::CRAFTY_JOKER,
        Joker::HALF_JOKER,
        Joker::CREDIT_CARD,
        Joker::BANNER,
        Joker::MYSTIC_SUMMIT,
        Joker::EIGHT_BALL,
        Joker::MISPRINT,
        Joker::RAISED_FIST,
        Joker::CHAOS_THE_CLOWN,
        Joker::SCARY_FACE,
        Joker::ABSTRACT_JOKER,
        Joker::DELAYED_GRATIFICATION,
        Joker::GROS_MICHEL,
        Joker::EVEN_STEVEN,
        Joker::ODD_TODD,
        Joker::SCHOLAR,
        Joker::BUSINESS_CARD,
        Joker::SUPERNOVA,
        Joker::RIDE_THE_BUS,
        Joker::EGG,
        Joker::RUNNER,
        Joker::ICE_CREAM,
        Joker::SPLASH,
        Joker::BLUE_JOKER,
        Joker::FACELESS_JOKER,
        Joker::GREEN_JOKER,
        Joker::SUPERPOSITION,
        Joker::TO_DO_LIST,
        Joker::CAVENDISH,
        Joker::RED_CARD,
        Joker::SQUARE_JOKER,
        Joker::RIFF_RAFF,
        Joker::PHOTOGRAPH,
        Joker::RESERVED_PARKING,
        Joker::MAIL_IN_REBATE,
        Joker::HALLUCINATION,
        Joker::FORTUNE_TELLER,
        Joker::JUGGLER,
        Joker::DRUNKARD,
        Joker::GOLDEN_JOKER,
        Joker::POPCORN,
        Joker::WALKIE_TALKIE,
        Joker::SMILEY_FACE,
        Joker::GOLDEN_TICKET,
        Joker::SWASHBUCKLER,
        Joker::HANGING_CHAD,
        Joker::SHOOT_THE_MOON,
    };
    
    constexpr std::array<Joker, 64> UNCOMMON_JOKERS = {
        Joker::JOKER_STENCIL,
        Joker::FOUR_FINGERS,
        Joker::MIME,
        Joker::CEREMONIAL_DAGGER,
        Joker::MARBLE_JOKER,
        Joker::LOYALTY_CARD,
        Joker::DUSK,
        Joker::FIBONACCI,
        Joker::STEEL_JOKER,
        Joker::HACK,
        Joker::PAREIDOLIA,
        Joker::SPACE_JOKER,
        Joker::BURGLAR,
        Joker::BLACKBOARD,
        Joker::SIXTH_SENSE,
        Joker::CONSTELLATION,
        Joker::HIKER,
        Joker::CARD_SHARP,
        Joker::MADNESS,
        Joker::SEANCE,
        Joker::VAMPIRE,
        Joker::SHORTCUT,
        Joker::HOLOGRAM,
        Joker::CLOUD_9,
        Joker::ROCKET,
        Joker::MIDAS_MASK,
        Joker::LUCHADOR,
        Joker::GIFT_CARD,
        Joker::TURTLE_BEAN,
        Joker::EROSION,
        Joker::TO_THE_MOON,
        Joker::STONE_JOKER,
        Joker::LUCKY_CAT,
        Joker::BULL,
        Joker::DIET_COLA,
        Joker::TRADING_CARD,
        Joker::FLASH_CARD,
        Joker::SPARE_TROUSERS,
        Joker::RAMEN,
        Joker::SELTZER,
        Joker::CASTLE,
        Joker::MR_BONES,
        Joker::ACROBAT,
        Joker::SOCK_AND_BUSKIN,
        Joker::TROUBADOUR,
        Joker::CERTIFICATE,
        Joker::SMEARED_JOKER,
        Joker::THROWBACK,
        Joker::ROUGH_GEM,
        Joker::BLOODSTONE,
        Joker::ARROWHEAD,
        Joker::ONYX_AGATE,
        Joker::GLASS_JOKER,
        Joker::SHOWMAN,
        Joker::FLOWER_POT,
        Joker::MERRY_ANDY,
        Joker::OOPS_ALL_6S,
        Joker::THE_IDOL,
        Joker::SEEING_DOUBLE,
        Joker::MATADOR,
        Joker::SATELLITE,
        Joker::CARTOMANCER,
        Joker::ASTRONOMER,
        Joker::BOOTSTRAPS,
    };
    
    constexpr std::array<Joker, 20> RARE_JOKERS = {
        Joker::DNA,
        Joker::VAGABOND,
        Joker::BARON,
        Joker::OBELISK,
        Joker::BASEBALL_CARD,
        Joker::ANCIENT_JOKER,
        Joker::CAMPFIRE,
        Joker::BLUEPRINT,
        Joker::WEE_JOKER,
        Joker::HIT_THE_ROAD,
        Joker::THE_DUO,
        Joker::THE_TRIO,
        Joker::THE_FAMILY,
        Joker::THE_ORDER,
        Joker::THE_TRIBE,
        Joker::STUNTMAN,
        Joker::INVISIBLE_JOKER,
        Joker::BRAINSTORM,
        Joker::DRIVERS_LICENSE,
        Joker::BURNT_JOKER,
    };
    
    constexpr std::array<Joker, 5> LEGENDARY_JOKERS = {
        Joker::CANIO,
        Joker::TRIBOULET,
        Joker::YORICK,
        Joker::CHICOT,
        Joker::PERKEO,
    };
    
    constexpr std::array<Tarot, 22> ALL_TAROTS = {
        Tarot::THE_FOOL,
        Tarot::THE_MAGICIAN,
        Tarot::THE_HIGH_PRIESTESS,
        Tarot::THE_EMPRESS,
        Tarot::THE_EMPEROR,
        Tarot::THE_HIEROPHANT,
        Tarot::THE_LOVERS,
        Tarot::THE_CHARIOT,
        Tarot::JUSTICE,
        Tarot::THE_HERMIT,
        Tarot::THE_WHEEL_OF_FORTUNE,
        Tarot::STRENGTH,
        Tarot::THE_HANGED_MAN,
        Tarot::DEATH,
        Tarot::TEMPERANCE,
        Tarot::THE_DEVIL,
        Tarot::THE_TOWER,
        Tarot::THE_STAR,
        Tarot::THE_MOON,
        Tarot::THE_SUN,
        Tarot::JUDGEMENT,
        Tarot::THE_WORLD,
    };
    
    constexpr std::array<Planet, 12> ALL_PLANETS = {
        Planet::MERCURY,
        Planet::VENUS,
        Planet::EARTH,
        Planet::MARS,
        Planet::JUPITER,
        Planet::SATURN,
        Planet::URANUS,
        Planet::NEPTUNE,
        Planet::PLUTO,
        Planet::PLANET_X,
        Planet::CERES,
        Planet::ERIS,
    };
    
    constexpr std::array<Spectral, 18> ALL_SPECTRALS = {
        Spectral::FAMILIAR,        // 0 - "Familiar"
        Spectral::GRIM,            // 1 - "Grim"
        Spectral::INCANTATION,     // 2 - "Incantation"
        Spectral::TALISMAN,        // 3 - "Talisman"
        Spectral::AURA,            // 4 - "Aura"
        Spectral::WRAITH,          // 5 - "Wraith"
        Spectral::SIGIL,           // 6 - "Sigil"
        Spectral::OUIJA,           // 7 - "Ouija"
        Spectral::ECTOPLASM,       // 8 - "Ectoplasm"
        Spectral::IMMOLATE,        // 9 - "Immolate"
        Spectral::ANKH,            // 10 - "Ankh"
        Spectral::DEJA_VU,         // 11 - "Deja Vu"
        Spectral::HEX,             // 12 - "Hex"
        Spectral::TRANCE,          // 13 - "Trance"
        Spectral::MEDIUM,          // 14 - "Medium"
        Spectral::CRYPTID,         // 15 - "Cryptid"
        Spectral::INVALID,         // 16 - "RETRY" equivalent
        Spectral::INVALID          // 17 - "RETRY" equivalent
    };
    
    constexpr std::array<Tag, 24> ALL_TAGS = {
        Tag::UNCOMMON_TAG,      // 0 - "Uncommon Tag"
        Tag::RARE_TAG,          // 1 - "Rare Tag" 
        Tag::NEGATIVE_TAG,      // 2 - "Negative Tag"
        Tag::FOIL_TAG,          // 3 - "Foil Tag"
        Tag::HOLOGRAPHIC_TAG,   // 4 - "Holographic Tag"
        Tag::POLYCHROME_TAG,    // 5 - "Polychrome Tag"
        Tag::INVESTMENT_TAG,    // 6 - "Investment Tag"
        Tag::VOUCHER_TAG,       // 7 - "Voucher Tag"
        Tag::BOSS_TAG,          // 8 - "Boss Tag"
        Tag::STANDARD_TAG,      // 9 - "Standard Tag"
        Tag::CHARM_TAG,         // 10 - "Charm Tag" - CRITICAL: This must be at index 10!
        Tag::METEOR_TAG,        // 11 - "Meteor Tag"
        Tag::BUFFOON_TAG,       // 12 - "Buffoon Tag"
        Tag::HANDY_TAG,         // 13 - "Handy Tag"
        Tag::GARBAGE_TAG,       // 14 - "Garbage Tag"
        Tag::ETHEREAL_TAG,      // 15 - "Ethereal Tag"
        Tag::COUPON_TAG,        // 16 - "Coupon Tag"
        Tag::DOUBLE_TAG,        // 17 - "Double Tag"
        Tag::JUGGLE_TAG,        // 18 - "Juggle Tag"
        Tag::D6_TAG,            // 19 - "D6 Tag"
        Tag::TOPUP_TAG,         // 20 - "Top-up Tag"
        Tag::SPEED_TAG,         // 21 - "Speed Tag"
        Tag::ORBITAL_TAG,       // 22 - "Orbital Tag"
        Tag::ECONOMY_TAG,       // 23 - "Economy Tag"
    };
    
    constexpr std::array<Boss, 28> ALL_BOSSES = {
        Boss::THE_ARM,
        Boss::THE_CLUB,
        Boss::THE_EYE,
        Boss::AMBER_ACORN,
        Boss::CERULEAN_BELL,
        Boss::CRIMSON_HEART,
        Boss::VERDANT_LEAF,
        Boss::VIOLET_VESSEL,
        Boss::THE_FISH,
        Boss::THE_FLINT,
        Boss::THE_GOAD,
        Boss::THE_HEAD,
        Boss::THE_HOOK,
        Boss::THE_HOUSE,
        Boss::THE_MANACLE,
        Boss::THE_MARK,
        Boss::THE_MOUTH,
        Boss::THE_NEEDLE,
        Boss::THE_OX,
        Boss::THE_PILLAR,
        Boss::THE_PLANT,
        Boss::THE_PSYCHIC,
        Boss::THE_SERPENT,
        Boss::THE_TOOTH,
        Boss::THE_WALL,
        Boss::THE_WATER,
        Boss::THE_WHEEL,
        Boss::THE_WINDOW,
    };

    constexpr std::array<Boss, 5> BOSSES_ANTE8 = {
        Boss::AMBER_ACORN,
        Boss::CERULEAN_BELL,
        Boss::CRIMSON_HEART,
        Boss::VERDANT_LEAF,
        Boss::VIOLET_VESSEL,
    };

    constexpr std::array<Boss, 23> BOSSES_OTHER_ANTES = {
        Boss::THE_ARM,
        Boss::THE_CLUB,
        Boss::THE_EYE,
        Boss::THE_FISH,
        Boss::THE_FLINT,
        Boss::THE_GOAD,
        Boss::THE_HEAD,
        Boss::THE_HOOK,
        Boss::THE_HOUSE,
        Boss::THE_MANACLE,
        Boss::THE_MARK,
        Boss::THE_MOUTH,
        Boss::THE_NEEDLE,
        Boss::THE_OX,
        Boss::THE_PILLAR,
        Boss::THE_PLANT,
        Boss::THE_PSYCHIC,
        Boss::THE_SERPENT,
        Boss::THE_TOOTH,
        Boss::THE_WALL,
        Boss::THE_WATER,
        Boss::THE_WHEEL,
        Boss::THE_WINDOW,
    };
    
    constexpr std::array<Voucher, 32> ALL_VOUCHERS = {
        Voucher::OVERSTOCK,         // 0 - "Overstock"
        Voucher::OVERSTOCK_PLUS,    // 1 - "Overstock Plus"
        Voucher::CLEARANCE_SALE,    // 2 - "Clearance Sale"
        Voucher::LIQUIDATION,       // 3 - "Liquidation"
        Voucher::HONE,              // 4 - "Hone"
        Voucher::GLOW_UP,           // 5 - "Glow Up"
        Voucher::REROLL_SURPLUS,    // 6 - "Reroll Surplus"
        Voucher::REROLL_GLUT,       // 7 - "Reroll Glut"
        Voucher::CRYSTAL_BALL,      // 8 - "Crystal Ball"
        Voucher::OMEN_GLOBE,        // 9 - "Omen Globe"
        Voucher::TELESCOPE,         // 10 - "Telescope"
        Voucher::OBSERVATORY,       // 11 - "Observatory"
        Voucher::GRABBER,           // 12 - "Grabber"
        Voucher::NACHO_TONG,        // 13 - "Nacho Tong"
        Voucher::WASTEFUL,          // 14 - "Wasteful"
        Voucher::RECYCLOMANCY,      // 15 - "Recyclomancy"
        Voucher::TAROT_MERCHANT,    // 16 - "Tarot Merchant"
        Voucher::TAROT_TYCOON,      // 17 - "Tarot Tycoon"
        Voucher::PLANET_MERCHANT,   // 18 - "Planet Merchant"
        Voucher::PLANET_TYCOON,     // 19 - "Planet Tycoon"
        Voucher::SEED_MONEY,        // 20 - "Seed Money"
        Voucher::MONEY_TREE,        // 21 - "Money Tree"
        Voucher::BLANK,             // 22 - "Blank"
        Voucher::ANTIMATTER,        // 23 - "Antimatter"
        Voucher::MAGIC_TRICK,       // 24 - "Magic Trick"
        Voucher::ILLUSION,          // 25 - "Illusion"
        Voucher::HIEROGLYPH,        // 26 - "Hieroglyph"
        Voucher::PETROGLYPH,        // 27 - "Petroglyph"
        Voucher::DIRECTORS_CUT,     // 28 - "Director's Cut"
        Voucher::RETCON,            // 29 - "Retcon"
        Voucher::PAINT_BRUSH,       // 30 - "Paint Brush"
        Voucher::PALETTE,           // 31 - "Palette"
    };

    constexpr std::array<Enhancement, static_cast<size_t>(Enhancement::COUNT) - 1> ALL_ENHANCEMENTS = {
        Enhancement::BONUS,
        Enhancement::MULT,
        Enhancement::WILD,
        Enhancement::GLASS,
        Enhancement::STEEL,
        Enhancement::STONE,
        Enhancement::GOLD,
        Enhancement::LUCKY
    };
    
    // Weighted pack structure for pack generation
    struct WeightedPackItem {
        Pack pack;
        double weight;
        
        constexpr WeightedPackItem(Pack p, double w) : pack(p), weight(w) {}
    };
    
    // Pack weights - EXACT same as original PACKS array
    constexpr std::array<WeightedPackItem, 16> ALL_PACKS = {
        WeightedPackItem(Pack::INVALID, 22.42),          // RETRY equivalent
        WeightedPackItem(Pack::ARCANA_PACK, 4),
        WeightedPackItem(Pack::JUMBO_ARCANA_PACK, 2),
        WeightedPackItem(Pack::MEGA_ARCANA_PACK, 0.5),
        WeightedPackItem(Pack::CELESTIAL_PACK, 4),
        WeightedPackItem(Pack::JUMBO_CELESTIAL_PACK, 2),
        WeightedPackItem(Pack::MEGA_CELESTIAL_PACK, 0.5),
        WeightedPackItem(Pack::STANDARD_PACK, 4),
        WeightedPackItem(Pack::JUMBO_STANDARD_PACK, 2),
        WeightedPackItem(Pack::MEGA_STANDARD_PACK, 0.5),
        WeightedPackItem(Pack::BUFFOON_PACK, 1.2),
        WeightedPackItem(Pack::JUMBO_BUFFOON_PACK, 0.6),
        WeightedPackItem(Pack::MEGA_BUFFOON_PACK, 0.15),
        WeightedPackItem(Pack::SPECTRAL_PACK, 0.6),
        WeightedPackItem(Pack::JUMBO_SPECTRAL_PACK, 0.3),
        WeightedPackItem(Pack::MEGA_SPECTRAL_PACK, 0.07)
    };
    
    // Utility type for compile-time enum validation
    template<typename EnumType>
    constexpr bool is_valid_enum(EnumType value) {
        return static_cast<std::underlying_type_t<EnumType>>(value) < 
               static_cast<std::underlying_type_t<EnumType>>(EnumType::COUNT);
    }


    
} // namespace Items