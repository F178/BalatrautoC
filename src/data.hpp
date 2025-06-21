#pragma once
#include <vector>
#include <string>
#include <utility>

// --- JOKERS ---
const std::vector<std::string> JOKER_NAMES = {
    "Joker", "Greedy Joker", "Lusty Joker", "Wrathful Joker", "Gluttonous Joker", "Jolly Joker", "Zany Joker", "Mad Joker", "Crazy Joker", "Droll Joker",
    "Sly Joker", "Willy Joker", "Clever Joker", "Devious Joker", "Crafty Joker", "Half Joker", "Joker Stencil", "Four Fingers", "Mime", "Credit Card",
    "Ceremonial Dagger", "Banner", "Mystic Summit", "Marble Joker", "Loyalty Card", "8 Ball", "Misprint", "Dusk", "Raised Fist", "Chaos the Clown",
    "Fibonacci", "Steel Joker", "Scary Face", "Abstract Joker", "Delayed Gratification", "Hack", "Pareidolia", "Gros Michel", "Even Steven", "Odd Todd",
    "Scholar", "Business Card", "Supernova", "Ride the Bus", "Space Joker", "Egg", "Burglar", "Blackboard", "Runner", "Ice Cream",
    "DNA", "Splash", "Blue Joker", "Sixth Sense", "Constellation", "Hiker", "Faceless Joker", "Green Joker", "Superposition", "To Do List",
    "Cavendish", "Card Sharp", "Red Card", "Madness", "Square Joker", "Seance", "Riff-raff", "Vampire", "Shortcut", "Hologram",
    "Vagabond", "Baron", "Cloud 9", "Rocket", "Obelisk", "Midas Mask", "Luchador", "Photograph", "Gift Card", "Turtle Bean",
    "Erosion", "Reserved Parking", "Mail-In Rebate", "To The Moon", "Hallucination", "Fortune Teller", "Juggler", "Drunkard", "Stone Joker", "Golden Joker",
    "Lucky Cat", "Baseball Card", "Bull", "Diet Cola", "Trading Card", "Flash Card", "Popcorn", "Spare Trousers", "Ancient Joker", "Ramen",
    "Walkie Talkie", "Seltzer", "Castle", "Smiley Face", "Campfire", "Golden Ticket", "Mr. Bones", "Acrobat", "Sock and Buskin", "Swashbuckler",
    "Troubadour", "Certificate", "Smeared Joker", "Throwback", "Hanging Chad", "Rough Gem", "Bloodstone", "Arrowhead", "Onyx Agate", "Glass Joker",
    "Showman", "Flower Pot", "Blueprint", "Wee Joker", "Merry Andy", "Oops! All 6s", "The Idol", "Seeing Double", "Matador", "Hit the Road",
    "The Duo", "The Trio", "The Family", "The Order", "The Tribe", "Stuntman", "Invisible Joker", "Brainstorm", "Satellite", "Shoot the Moon",
    "Driver's License", "Cartomancer", "Astronomer", "Burnt Joker", "Bootstraps", "Caino", "Triboulet", "Yorick", "Chicot", "Perkeo"
};

// --- DECKS ---
const std::vector<std::string> DECK_NAMES = {
    "Red Deck", "Blue Deck", "Yellow Deck", "Green Deck", "Black Deck", "Magic Deck", "Nebula Deck", "Ghost Deck", "Abandoned Deck", "Checkered Deck",
    "Zodiac Deck", "Paint Deck", "Anaglyph Deck", "Erratic Deck", "Plasma Deck"
};

// --- STAKES ---
const std::vector<std::string> STAKE_NAMES = {
    "White Stake", "Red Stake", "Green Stake", "Black Stake", "Blue Stake", "Purple Stake", "Orange Stake", "Gold Stake"
};

// --- CATEGORY PAIRS ---
const std::vector<std::pair<std::string, const std::vector<std::string>&>> CATEGORIES = {
    {"jokers", JOKER_NAMES},
    {"decks", DECK_NAMES},
};
