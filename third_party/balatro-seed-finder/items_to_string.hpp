#pragma once

#include "items.hpp"

namespace Items {

    constexpr std::array<const char*, static_cast<size_t>(Items::Joker::COUNT)> JOKER_NAMES = {
        // Common Items::Jokers (0-60)
        "Joker", "Greedy Joker", "Lusty Joker", "Wrathful Joker", "Gluttonous Joker",
        "Jolly Joker", "Zany Joker", "Mad Joker", "Crazy Joker", "Droll Joker",
        "Sly Joker", "Wily Joker", "Clever Joker", "Devious Joker", "Crafty Joker",
        "Half Joker", "Credit Card", "Banner", "Mystic Summit", "8 Ball",
        "Misprint", "Raised Fist", "Chaos the Clown", "Scary Face", "Abstract Joker",
        "Delayed Gratification", "Gros Michel", "Even Steven", "Odd Todd", "Scholar",
        "Business Card", "Supernova", "Ride the Bus", "Egg", "Runner",
        "Ice Cream", "Splash", "Blue Joker", "Faceless Joker", "Green Joker",
        "Superposition", "To Do List", "Cavendish", "Red Card", "Square Joker",
        "Riff-raff", "Photograph", "Reserved Parking", "Mail In Rebate", "Hallucination",
        "Fortune Teller", "Juggler", "Drunkard", "Golden Joker", "Popcorn",
        "Walkie Talkie", "Smiley Face", "Golden Ticket", "Swashbuckler", "Hanging Chad",
        "Shoot the Moon",

        // Uncommon Items::Jokers (61-124)
        "Joker Stencil", "Four Fingers", "Mime", "Ceremonial Dagger", "Marble Joker",
        "Loyalty Card", "Dusk", "Fibonacci", "Steel Joker", "Hack",
        "Pareidolia", "Space Joker", "Burglar", "Blackboard", "Sixth Sense",
        "Constellation", "Hiker", "Card Sharp", "Madness", "Seance",
        "Vampire", "Shortcut", "Hologram", "Cloud 9", "Rocket",
        "Midas Mask", "Luchador", "Gift Card", "Turtle Bean", "Erosion",
        "To the Moon", "Stone Joker", "Lucky Cat", "Bull", "Diet Cola",
        "Trading Card", "Flash Card", "Spare Trousers", "Ramen", "Seltzer",
        "Castle", "Mr. Bones", "Acrobat", "Sock and Buskin", "Troubadour",
        "Certificate", "Smeared Joker", "Throwback", "Rough Gem", "Bloodstone",
        "Arrowhead", "Onyx Agate", "Glass Joker", "Showman", "Flower Pot",
        "Merry Andy", "Oops! All 6s", "The Idol", "Seeing Double", "Matador",
        "Satellite", "Cartomancer", "Astronomer", "Bootstraps",

        // Rare Items::Jokers (125-144)
        "DNA", "Vagabond", "Baron", "Obelisk", "Baseball Card",
        "Ancient Joker", "Campfire", "Blueprint", "Wee Joker", "Hit the Road",
        "The Duo", "The Trio", "The Family", "The Order", "The Tribe",
        "Stuntman", "Invisible Joker", "Brainstorm", "Drivers License", "Burnt Joker",

        // Legendary Items::Jokers (145-149)
        "Canio", "Triboulet", "Yorick", "Chicot", "Perkeo"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Tag::COUNT)> TAG_NAMES = {
        "Uncommon Tag", "Rare Tag", "Negative Tag", "Foil Tag", "Holographic Tag",
        "Polychrome Tag", "Investment Tag", "Voucher Tag", "Boss Tag", "Standard Tag",
        "Charm Tag", "Meteor Tag", "Buffoon Tag", "Handy Tag", "Garbage Tag",
        "Ethereal Tag", "Coupon Tag", "Double Tag", "Juggle Tag", "D6 Tag",
        "Top-up Tag", "Speed Tag", "Orbital Tag", "Economy Tag"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Tarot::COUNT)> TAROT_NAMES = {
        "The Fool", "The Magician", "The High Priestess", "The Empress", "The Emperor",
        "The Hierophant", "The Lovers", "The Chariot", "Justice", "The Hermit",
        "The Wheel of Fortune", "Strength", "The Hanged Man", "Death", "Temperance",
        "The Devil", "The Tower", "The Star", "The Moon", "The Sun",
        "Judgement", "The World", "The Soul"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Planet::COUNT)> PLANET_NAMES = {
        "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn",
        "Uranus", "Neptune", "Pluto", "Planet X", "Ceres", "Eris", "Black Hole"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Spectral::COUNT)> SPECTRAL_NAMES = {
        "Familiar", "Grim", "Incantation", "Talisman", "Aura", "Wraith",
        "Sigil", "Ouija", "Ectoplasm", "Immolate", "Ankh", "Deja Vu",
        "Hex", "Trance", "Medium", "Cryptid", "The Soul", "Black Hole"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Boss::COUNT)> BOSS_NAMES = {
        "The Arm", "The Club", "The Eye", "Amber Acorn", "Cerulean Bell",
        "Crimson Heart", "Verdant Leaf", "Violet Vessel", "The Fish", "The Flint",
        "The Goad", "The Head", "The Hook", "The House", "The Manacle",
        "The Mark", "The Mouth", "The Needle", "The Ox", "The Pillar",
        "The Plant", "The Psychic", "The Serpent", "The Tooth", "The Wall",
        "The Water", "The Wheel", "The Window"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Voucher::COUNT)> VOUCHER_NAMES = {
        "Overstock", "Overstock Plus", "Clearance Sale", "Liquidation", "Hone",
        "Glow Up", "Reroll Surplus", "Reroll Glut", "Crystal Ball", "Omen Globe",
        "Telescope", "Observatory", "Grabber", "Nacho Tong", "Wasteful",
        "Recyclomancy", "Tarot Merchant", "Tarot Tycoon", "Planet Merchant", "Planet Tycoon",
        "Seed Money", "Money Tree", "Blank", "Antimatter", "Magic Trick",
        "Illusion", "Hieroglyph", "Petroglyph", "Director's Cut", "Retcon",
        "Paint Brush", "Palette"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Edition::COUNT)> EDITION_NAMES = {
        "No Edition", "Foil", "Holographic", "Polychrome", "Negative"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Enhancement::COUNT)> ENHANCEMENT_NAMES = {
        "No Enhancement", "Bonus", "Mult", "Wild", "Glass", "Steel", "Stone", "Gold", "Lucky"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Seal::COUNT)> SEAL_NAMES = {
        "No Seal", "Red Seal", "Blue Seal", "Gold Seal", "Purple Seal"
    };

    constexpr std::array<const char*, static_cast<size_t>(Items::Pack::COUNT)> PACK_NAMES = {
        "Arcana Pack", "Jumbo Arcana Pack", "Mega Arcana Pack",
        "Celestial Pack", "Jumbo Celestial Pack", "Mega Celestial Pack",
        "Standard Pack", "Jumbo Standard Pack", "Mega Standard Pack",
        "Buffoon Pack", "Jumbo Buffoon Pack", "Mega Buffoon Pack",
        "Spectral Pack", "Jumbo Spectral Pack", "Mega Spectral Pack"
    };

    constexpr const char* toString(Items::Joker joker) {
        auto idx = static_cast<size_t>(joker);
        return (idx < JOKER_NAMES.size()) ? JOKER_NAMES[idx] : "Invalid Joker";
    }

    constexpr const char* toString(Items::Tag tag) {
        auto idx = static_cast<size_t>(tag);
        return (idx < TAG_NAMES.size()) ? TAG_NAMES[idx] : "Invalid Tag";
    }

    constexpr const char* toString(Items::Tarot tarot) {
        auto idx = static_cast<size_t>(tarot);
        return (idx < TAROT_NAMES.size()) ? TAROT_NAMES[idx] : "Invalid Tarot";
    }

    constexpr const char* toString(Items::Planet planet) {
        auto idx = static_cast<size_t>(planet);
        return (idx < PLANET_NAMES.size()) ? PLANET_NAMES[idx] : "Invalid Planet";
    }

    constexpr const char* toString(Items::Spectral spectral) {
        auto idx = static_cast<size_t>(spectral);
        return (idx < SPECTRAL_NAMES.size()) ? SPECTRAL_NAMES[idx] : "Invalid Spectral";
    }

    constexpr const char* toString(Items::Boss boss) {
        auto idx = static_cast<size_t>(boss);
        return (idx < BOSS_NAMES.size()) ? BOSS_NAMES[idx] : "Invalid Boss";
    }

    constexpr const char* toString(Items::Voucher voucher) {
        auto idx = static_cast<size_t>(voucher);
        return (idx < VOUCHER_NAMES.size()) ? VOUCHER_NAMES[idx] : "Invalid Voucher";
    }

    constexpr const char* toString(Items::Edition edition) {
        auto idx = static_cast<size_t>(edition);
        return (idx < EDITION_NAMES.size()) ? EDITION_NAMES[idx] : "Invalid Edition";
    }

    constexpr const char* toString(Items::Enhancement enhancement) {
        auto idx = static_cast<size_t>(enhancement);
        return (idx < ENHANCEMENT_NAMES.size()) ? ENHANCEMENT_NAMES[idx] : "Invalid Enhancement";
    }

    constexpr const char* toString(Items::Seal seal) {
        auto idx = static_cast<size_t>(seal);
        return (idx < SEAL_NAMES.size()) ? SEAL_NAMES[idx] : "Invalid Seal";
    }

    constexpr const char* toString(Items::Pack pack) {
        auto idx = static_cast<size_t>(pack);
        return (idx < PACK_NAMES.size()) ? PACK_NAMES[idx] : "Invalid Pack";
    }
}
