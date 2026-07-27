#pragma once

#include "items_utils.hpp"
#include "items.hpp"
#include "items_choice.hpp"
#include "rand_util.hpp"
#include "bitmap_lock.hpp"
#include <unordered_map>
// #include <map>
#include <array>

// High-performance enum-based Instance class
// Replaces expensive string operations with fast integer operations

namespace Instance {
    
    // ========================================
    // OPTIMIZED INSTANCE CLASS
    // ========================================
    
    class Instance {
    private:
        std::string seed;
        double hashedSeed;
        std::unordered_map<std::string, double> nodeCache;
        Locks::EnumLockSystem enumLocks;
        LuaRandom rng;
        
        // Instance parameters
        std::string deck;
        std::string stake;
        bool showman;
        int sixesFactor;
        long version;
        
        // Cache for generated first pack
        bool generatedFirstPack;
        
        // Fast node computation with caching
        inline double get_node(const std::string& ID) {
            // Optimize: Use find() to avoid double lookup and pre-allocate string
            auto it = nodeCache.find(ID);
            if (it == nodeCache.end()) {
                // Pre-allocate string to avoid reallocations
                std::string combined;
                combined.reserve(ID.length() + seed.length());
                combined = ID + seed;
                // Insert directly with computed hash
                it = nodeCache.emplace(ID, pseudohash(combined)).first;
            }
            
            // Update the cached value in-place - optimized fmod(x, 1) = x - floor(x)
            double temp = it->second * 1.72431234 + 2.134453429141;
            it->second = round13(temp - std::floor(temp));
            return (it->second + hashedSeed) / 2;
        }
        
        // Fast random generation
        double random(const std::string& ID) {
            rng = LuaRandom(get_node(ID));
            return rng.random();
        }
        
        int randint(const std::string& ID, int min, int max) {
            rng = LuaRandom(get_node(ID));
            return rng.randint(min, max);
        }
        
    public:
        Instance(const std::string& s) 
            : seed(s), hashedSeed(pseudohash(s)), generatedFirstPack(false) {
            deck = "Red Deck";
            stake = "White Stake";
            showman = false;
            sixesFactor = 1;
            version = 10106;
            rng = LuaRandom(0);
        }
        
        // ========================================
        // OPTIMIZED ENUM-BASED GENERATORS
        // ========================================
        
        // CRITICAL HOT PATH: Ultra-fast tarot generation
        Items::Tarot nextTarot_enum(const std::string& source, int ante, bool soulable = false) {
            std::string anteStr = std::to_string(ante);
            
            // Fast soul card check with direct enum comparison
            if (soulable && (showman || !enumLocks.isLocked(Items::Tarot::SPECIAL_THE_SOUL))) {
                if (random("soul_Tarot" + anteStr) > 0.997) {
                    return Items::Tarot::SPECIAL_THE_SOUL;
                }
            }
            
            // Fast enum-based selection - no string operations
            auto get_node_func = [this](const std::string& id) { return get_node(id); };
            return Items::TarotChoice(get_node_func, enumLocks, showman, "Tarot" + source + anteStr);
        }
        
        // CRITICAL HOT PATH: Ultra-fast planet generation  
        Items::Planet nextPlanet_enum(const std::string& source, int ante, bool soulable = false) {
            std::string anteStr = std::to_string(ante);
            
            // Fast black hole check with direct enum comparison
            if (soulable && (showman || !enumLocks.isLocked(Items::Planet::SPECIAL_BLACK_HOLE))) {
                if (random("soul_Planet" + anteStr) > 0.997) {
                    return Items::Planet::SPECIAL_BLACK_HOLE;
                }
            }
            
            // Fast enum-based selection
            auto get_node_func = [this](const std::string& id) { return get_node(id); };
            return Items::PlanetChoice(get_node_func, enumLocks, showman, "Planet" + source + anteStr);
        }
        
        // CRITICAL HOT PATH: Ultra-fast spectral generation
        Items::Spectral nextSpectral_enum(const std::string& source, int ante, bool soulable = false) {
            std::string anteStr = std::to_string(ante);
            
            if (soulable) {
                // EXACT SAME LOGIC as original: Use single forcedKey variable
                Items::Spectral forcedKey = Items::Spectral::INVALID;
                if ((showman || !enumLocks.isLocked(Items::Spectral::SPECTRAL_THE_SOUL)) && random("soul_Spectral" + anteStr) > 0.997) {
                    forcedKey = Items::Spectral::SPECTRAL_THE_SOUL;
                }
                if ((showman || !enumLocks.isLocked(Items::Spectral::SPECTRAL_BLACK_HOLE)) && random("soul_Spectral" + anteStr) > 0.997) {
                    forcedKey = Items::Spectral::SPECTRAL_BLACK_HOLE;
                }
                if (forcedKey != Items::Spectral::INVALID) return forcedKey;
            }
            
            // Fast enum-based selection
            auto get_node_func = [this](const std::string& id) { return get_node(id); };
            return Items::SpectralChoice(get_node_func, enumLocks, showman, "Spectral" + source + anteStr);
        }
        
        // CRITICAL HOT PATH: Ultra-fast joker generation
        Items::OptimizedJokerData nextJoker_enum(const std::string& source, int ante, bool hasStickers = false) {
            std::string anteStr = std::to_string(ante);
            
            // Fast rarity determination
            uint8_t rarity;
            if (source == "sou") rarity = 4;
            else if (source == "wra") rarity = 3;
            else if (source == "rta") rarity = 3;
            else if (source == "uta") rarity = 2;
            else {
                double rarityPoll = random("rarity" + anteStr + source);
                if (rarityPoll > 0.95) rarity = 3;
                else if (rarityPoll > 0.7) rarity = 2;
                else rarity = 1;
            }
            
            // Fast edition determination
            Items::Edition edition = Items::Edition::NO_EDITION;
            int editionRate = 1;
            if (enumLocks.isVoucherActive(Items::Voucher::GLOW_UP)) editionRate = 4;
            else if (enumLocks.isVoucherActive(Items::Voucher::HONE)) editionRate = 2;
            double editionPoll = random("edi" + source + anteStr);
            if (editionPoll > 0.997) edition = Items::Edition::NEGATIVE;
            else if (editionPoll > 1 - 0.006 * editionRate) edition = Items::Edition::POLYCHROME;
            else if (editionPoll > 1 - 0.02 * editionRate) edition = Items::Edition::HOLOGRAPHIC;
            else if (editionPoll > 1 - 0.04 * editionRate) edition = Items::Edition::FOIL;
            
            // MAJOR SPEEDUP: Fast joker selection by rarity using enum arrays
            auto get_node_func = [this](const std::string& id) { return get_node(id); };
            Items::Joker joker;
            
            switch (rarity) {
                case 4:
                    if (version > 10099) {
                        joker = Items::LegendaryJokerChoice(get_node_func, enumLocks, showman, "Joker4");
                    } else {
                        joker = Items::LegendaryJokerChoice(get_node_func, enumLocks, showman, "Joker" + std::to_string(rarity) + source + anteStr);
                    }
                    break;
                case 3:
                    joker = Items::RareJokerChoice(get_node_func, enumLocks, showman, "Joker" + std::to_string(rarity) + source + anteStr);
                    break;
                case 2:
                    joker = Items::UncommonJokerChoice(get_node_func, enumLocks, showman, "Joker" + std::to_string(rarity) + source + anteStr);
                    break;
                default:
                    joker = Items::CommonJokerChoice(get_node_func, enumLocks, showman, "Joker" + std::to_string(rarity) + source + anteStr);
                    break;
            }
            
            // Fast sticker determination with enum-based joker checking
            bool eternal = false, perishable = false, rental = false;
            if (hasStickers) {
                if (version > 10103) {
                    double stickerPoll = random(((source=="buf") ? "packetper" : "etperpoll") + anteStr);
                    
                    // Eternal sticker logic with fast enum-based exclusion checking
                    if (stickerPoll > 0.7 && (stake == "Black Stake" || stake == "Blue Stake" || 
                                             stake == "Purple Stake" || stake == "Orange Stake" || stake == "Gold Stake")) {
                        // MAJOR SPEEDUP: Fast enum comparison instead of multiple string comparisons
                        bool canBeEternal = !(joker == Items::Joker::GROS_MICHEL || joker == Items::Joker::ICE_CREAM || 
                                            joker == Items::Joker::CAVENDISH || joker == Items::Joker::LUCHADOR ||
                                            joker == Items::Joker::TURTLE_BEAN || joker == Items::Joker::DIET_COLA || 
                                            joker == Items::Joker::POPCORN || joker == Items::Joker::RAMEN ||
                                            joker == Items::Joker::SELTZER || joker == Items::Joker::MR_BONES || 
                                            joker == Items::Joker::INVISIBLE_JOKER);
                        eternal = canBeEternal;
                    }
                    
                    // Perishable sticker logic with fast enum-based exclusion checking
                    if ((stickerPoll > 0.4 && stickerPoll <= 0.7) && (stake == "Orange Stake" || stake == "Gold Stake")) {
                        // MAJOR SPEEDUP: Fast enum comparison instead of multiple string comparisons
                        bool canBePerishable = !(joker == Items::Joker::CEREMONIAL_DAGGER || joker == Items::Joker::RIDE_THE_BUS || 
                                                joker == Items::Joker::RUNNER || joker == Items::Joker::CONSTELLATION ||
                                                joker == Items::Joker::GREEN_JOKER || joker == Items::Joker::RED_CARD || 
                                                joker == Items::Joker::MADNESS || joker == Items::Joker::SQUARE_JOKER ||
                                                joker == Items::Joker::VAMPIRE || joker == Items::Joker::ROCKET || 
                                                joker == Items::Joker::OBELISK || joker == Items::Joker::LUCKY_CAT ||
                                                joker == Items::Joker::FLASH_CARD || joker == Items::Joker::SPARE_TROUSERS || 
                                                joker == Items::Joker::CASTLE || joker == Items::Joker::WEE_JOKER);
                        perishable = canBePerishable;
                    }
                    
                    // Rental sticker logic
                    if (stake == "Gold Stake") {
                        rental = random(((source=="buf") ? "packssjr" : "ssjr") + anteStr) > 0.7;
                    }
                } else {
                    // Legacy version sticker logic
                    if (stake == "Black Stake" || stake == "Blue Stake" || stake == "Purple Stake" || 
                        stake == "Orange Stake" || stake == "Gold Stake") {
                        // MAJOR SPEEDUP: Fast enum comparison for eternal exclusions
                        bool canBeEternal = !(joker == Items::Joker::GROS_MICHEL || joker == Items::Joker::ICE_CREAM || 
                                            joker == Items::Joker::CAVENDISH || joker == Items::Joker::LUCHADOR ||
                                            joker == Items::Joker::TURTLE_BEAN || joker == Items::Joker::DIET_COLA || 
                                            joker == Items::Joker::POPCORN || joker == Items::Joker::RAMEN ||
                                            joker == Items::Joker::SELTZER || joker == Items::Joker::MR_BONES || 
                                            joker == Items::Joker::INVISIBLE_JOKER);
                        if (canBeEternal) {
                            eternal = random("stake_shop_joker_eternal" + anteStr) > 0.7;
                        }
                    }
                    
                    if (version > 10099) {
                        if ((stake == "Orange Stake" || stake == "Gold Stake") && !eternal) {
                            perishable = random("ssjp" + anteStr) > 0.49;
                        }
                        if (stake == "Gold Stake") {
                            rental = random("ssjr" + anteStr) > 0.7;
                        }
                    }
                }
            }
            
            return Items::OptimizedJokerData(joker, rarity, edition, eternal, perishable, rental);
        }
        
        // CRITICAL HOT PATH: Ultra-fast tag generation
        Items::Tag nextTag_enum(int ante) {
            auto get_node_func = [this](const std::string& id) { return get_node(id); };
            return Items::TagChoice(get_node_func, enumLocks, showman, "Tag" + std::to_string(ante));
        }
        
        // CRITICAL HOT PATH: Ultra-fast voucher generation
        Items::Voucher nextVoucher_enum(int ante) {
            auto get_node_func = [this](const std::string& id) { return get_node(id); };
            return Items::VoucherChoice(get_node_func, enumLocks, showman, "Voucher" + std::to_string(ante));
        }
        
        // CRITICAL HOT PATH: Ultra-fast boss generation
        Items::Boss nextBoss_enum(int ante) {
            // Fast boss pool construction using enum arrays
            std::vector<Items::Boss> bossPool;
            bool needsTemple = (ante % 8 == 0);

            // Use the appropriate boss array based on ante
            if (needsTemple) {
                // Ante 8: Use non-temple bosses (BOSSES_ANTE8)
                for (const Items::Boss& boss : Items::BOSSES_ANTE8) {
                    if (!enumLocks.isLocked(boss)) {
                        bossPool.push_back(boss);
                    }
                }
            } else {
                // Other antes: Use temple bosses (BOSSES_OTHER_ANTES)
                for (const Items::Boss& boss : Items::BOSSES_OTHER_ANTES) {
                    if (!enumLocks.isLocked(boss)) {
                        bossPool.push_back(boss);
                    }
                }
            }

            if (bossPool.empty()) {
                // Unlock all valid bosses
                if (needsTemple) {
                    for (const Items::Boss& boss : Items::BOSSES_ANTE8) {
                        enumLocks.unlock(boss);
                    }
                } else {
                    for (const Items::Boss& boss : Items::BOSSES_OTHER_ANTES) {
                        enumLocks.unlock(boss);
                    }
                }
                return nextBoss_enum(ante); // Recurse
            }

            // Fast selection from pool
            LuaRandom rng(get_node("boss"));
            Items::Boss chosenBoss = bossPool[rng.randint(0, bossPool.size() - 1)];
            enumLocks.lock(chosenBoss);

            return chosenBoss;
        }

        // CRITICAL HOT PATH: Ultra-fast shop item generation
        Items::OptimizedShopItem nextShopItem_enum(int ante) {
            std::string anteStr = std::to_string(ante);
            
            // Fast shop rate calculation (simplified)
            double jokerRate = 20, tarotRate = 4, planetRate = 4;
            double playingCardRate = 0, spectralRate = 0;

            if (deck == "Ghost Deck") {
                spectralRate = 2;
            }
            if (enumLocks.isVoucherActive(Items::Voucher::TAROT_TYCOON)) {
                tarotRate = 32;
            } else if (enumLocks.isVoucherActive(Items::Voucher::TAROT_MERCHANT)) {
                tarotRate = 9.6;
            }
            if (enumLocks.isVoucherActive(Items::Voucher::PLANET_TYCOON)) {
                planetRate = 32;
            } else if (enumLocks.isVoucherActive(Items::Voucher::PLANET_MERCHANT)) {
                planetRate = 9.6;
            }
            if (enumLocks.isVoucherActive(Items::Voucher::MAGIC_TRICK)) {
                playingCardRate = 4;
            }
            
            double totalRate = jokerRate + tarotRate + planetRate + playingCardRate + spectralRate;
            
            double cdtPoll = random("cdt" + anteStr) * totalRate;
            
            if (cdtPoll < jokerRate) {
                auto jokerData = nextJoker_enum("sho", ante, true);
                return Items::OptimizedShopItem(jokerData.joker, jokerData);
            } else {
                cdtPoll -= jokerRate;
                if (cdtPoll < tarotRate) {
                    return Items::OptimizedShopItem(nextTarot_enum("sho", ante, false));
                } else {
                    cdtPoll -= tarotRate;
                    if (cdtPoll < planetRate) {
                        return Items::OptimizedShopItem(nextPlanet_enum("sho", ante, false));
                    } else {
                        return Items::OptimizedShopItem(nextSpectral_enum("sho", ante, false));
                    }
                }
            }
        }
        
        // ========================================
        // PACK GENERATION (Enum-based)
        // ========================================
        
        Items::MixedArcanaPack nextArcanaPack_enum(int size, int ante) {
            Items::MixedArcanaPack pack;
            pack.tarots.reserve(size);
            pack.spectrals.reserve(size);
            pack.isSpectral.reserve(size);
            
            for (int i = 0; i < size; i++) {
                if (enumLocks.isVoucherActive(Items::Voucher::OMEN_GLOBE) && random("omen_globe") > 0.8) {
                    // Omen Globe effect: Generate spectral instead of tarot
                    auto spectral = nextSpectral_enum("ar2", ante, true);
                    pack.spectrals.push_back(spectral);
                    pack.tarots.push_back(Items::Tarot::INVALID); // Placeholder
                    pack.isSpectral.push_back(true);
                    if (!showman) enumLocks.lock(spectral);
                } else {
                    // Normal tarot card
                    auto tarot = nextTarot_enum("ar1", ante, true);
                    pack.tarots.push_back(tarot);
                    pack.spectrals.push_back(Items::Spectral::INVALID); // Placeholder
                    pack.isSpectral.push_back(false);
                    if (!showman) enumLocks.lock(tarot);
                }
            }
            
            // Unlock after generation
            for (int i = 0; i < size; i++) {
                if (pack.isSpectral[i]) {
                    enumLocks.unlock(pack.spectrals[i]);
                } else {
                    enumLocks.unlock(pack.tarots[i]);
                }
            }
            
            return pack;
        }
        
        std::vector<Items::Planet> nextCelestialPack_enum(int size, int ante) {
            std::vector<Items::Planet> pack;
            pack.reserve(size);
            
            for (int i = 0; i < size; i++) {
                pack.push_back(nextPlanet_enum("pl1", ante, true));
                if (!showman) enumLocks.lock(pack[i]);
            }
            
            for (int i = 0; i < size; i++) {
                enumLocks.unlock(pack[i]);
            }
            
            return pack;
        }
        
        std::vector<Items::Spectral> nextSpectralPack_enum(int size, int ante) {
            std::vector<Items::Spectral> pack;
            pack.reserve(size);
            
            for (int i = 0; i < size; i++) {
                pack.push_back(nextSpectral_enum("spe", ante, true));
                if (!showman) enumLocks.lock(pack[i]);
            }
            
            for (int i = 0; i < size; i++) {
                enumLocks.unlock(pack[i]);
            }
            
            return pack;
        }
        
        std::vector<Items::OptimizedJokerData> nextBuffoonPack_enum(int size, int ante) {
            std::vector<Items::OptimizedJokerData> pack;
            pack.reserve(size);
            
            for (int i = 0; i < size; i++) {
                pack.push_back(nextJoker_enum("buf", ante, true));
                if (!showman) enumLocks.lock(pack[i].joker);
            }
            
            for (int i = 0; i < size; i++) {
                enumLocks.unlock(pack[i].joker);
            }
            
            return pack;
        }
        
        // CRITICAL HOT PATH: Ultra-fast standard card generation
        Items::CardEnum nextStandardCard_enum(int ante) {
            std::string anteStr = std::to_string(ante);

            // Enhancement determination - EXACT SAME LOGIC as original
            Items::Enhancement enhancement;
            if (random("stdset" + anteStr) <= 0.6) {
                enhancement = Items::Enhancement::NO_ENHANCEMENT;
            } else {
                auto get_node_func = [this](const std::string& id) { return get_node(id); };
                enhancement = Items::EnhancementChoice(get_node_func, "Enhancedsta" + anteStr);
            }

            // Base card selection (keep as string - CARDS array is complex)
            std::string base = Items::CARDS[randint("frontsta" + anteStr, 0, Items::CARDS.size() - 1)];

            // Items::Edition determination - EXACT SAME LOGIC as original
            Items::Edition edition;
            double editionPoll = random("standard_edition" + anteStr);
            if (editionPoll > 0.988) edition = Items::Edition::POLYCHROME;
            else if (editionPoll > 0.96) edition = Items::Edition::HOLOGRAPHIC;
            else if (editionPoll > 0.92) edition = Items::Edition::FOIL;
            else edition = Items::Edition::NO_EDITION;

            // Seal determination - EXACT SAME LOGIC as original (no randchoice!)
            Items::Seal seal;
            if (random("stdseal" + anteStr) <= 0.8) {
                seal = Items::Seal::NO_SEAL;
            } else {
                double sealPoll = random("stdsealtype" + anteStr);
                if (sealPoll > 0.75) seal = Items::Seal::RED_SEAL;
                else if (sealPoll > 0.5) seal = Items::Seal::BLUE_SEAL;
                else if (sealPoll > 0.25) seal = Items::Seal::GOLD_SEAL;
                else seal = Items::Seal::PURPLE_SEAL;
            }

            return Items::CardEnum(base, enhancement, edition, seal);
        }
        
        // CRITICAL HOT PATH: Ultra-fast standard pack generation
        std::vector<Items::CardEnum> nextStandardPack_enum(int size, int ante) {
            std::vector<Items::CardEnum> pack;
            pack.reserve(size);
            
            for (int i = 0; i < size; i++) {
                pack.push_back(nextStandardCard_enum(ante));
            }
            
            return pack;
        }
        
        // CRITICAL HOT PATH: Ultra-fast pack generation
        Items::Pack nextPack_enum(int ante) {
            // EXACT SAME LOGIC as original nextPack
            if (ante <= 2 && !generatedFirstPack && version > 10099) {
                generatedFirstPack = true;
                return Items::Pack::BUFFOON_PACK;
            }
            
            std::string anteStr = std::to_string(ante);
            auto get_node_func = [this](const std::string& id) { return get_node(id); };
            
            // Fast weighted choice with retry logic
            Items::Pack chosen_pack;
            int resample = 1;
            do {
                if (resample == 1) {
                    chosen_pack = Items::PackChoice(get_node_func, "shop_pack" + anteStr);
                } else {
                    chosen_pack = Items::PackChoice(get_node_func, "shop_pack" + anteStr + "_resample" + std::to_string(resample));
                }
                resample++;
            } while (chosen_pack == Items::Pack::INVALID && resample <= 1000);
            
            return (chosen_pack == Items::Pack::INVALID) ? Items::Pack::ARCANA_PACK : chosen_pack;
        }
        
        // ========================================
        // LOCK MANAGEMENT
        // ========================================
        
        void initLocks(int ante, bool freshProfile, bool freshRun) {
            // Initialize locks using pre-computed global locks
            enumLocks.resetAll();
            
            // Apply the EXACT same locks as the original initInstance() for compatibility
            enumLocks.lock(Items::Voucher::OVERSTOCK_PLUS);
            enumLocks.lock(Items::Voucher::LIQUIDATION);
            enumLocks.lock(Items::Voucher::GLOW_UP);
            enumLocks.lock(Items::Voucher::REROLL_GLUT);
            enumLocks.lock(Items::Voucher::OMEN_GLOBE);
            enumLocks.lock(Items::Voucher::OBSERVATORY);
            enumLocks.lock(Items::Voucher::NACHO_TONG);
            enumLocks.lock(Items::Voucher::RECYCLOMANCY);
            enumLocks.lock(Items::Voucher::TAROT_TYCOON);
            enumLocks.lock(Items::Voucher::PLANET_TYCOON);
            enumLocks.lock(Items::Voucher::MONEY_TREE);
            enumLocks.lock(Items::Voucher::ANTIMATTER);
            enumLocks.lock(Items::Voucher::ILLUSION);
            enumLocks.lock(Items::Voucher::PETROGLYPH);
            enumLocks.lock(Items::Voucher::RETCON);
            enumLocks.lock(Items::Voucher::PALETTE);
            // Fast enum-based lock initialization
            if (ante < 2) {
                enumLocks.lock(Items::Boss::THE_FISH);
                enumLocks.lock(Items::Boss::THE_WALL);
                enumLocks.lock(Items::Boss::THE_HOUSE);
                enumLocks.lock(Items::Boss::THE_MARK);
                enumLocks.lock(Items::Boss::THE_WHEEL);
                enumLocks.lock(Items::Boss::THE_ARM);
                enumLocks.lock(Items::Boss::THE_WATER);
                enumLocks.lock(Items::Boss::THE_NEEDLE);
                enumLocks.lock(Items::Boss::THE_FLINT);
                enumLocks.lock(Items::Tag::NEGATIVE_TAG);
                enumLocks.lock(Items::Tag::STANDARD_TAG);
                enumLocks.lock(Items::Tag::METEOR_TAG);
                enumLocks.lock(Items::Tag::BUFFOON_TAG);
                enumLocks.lock(Items::Tag::HANDY_TAG);
                enumLocks.lock(Items::Tag::GARBAGE_TAG);
                enumLocks.lock(Items::Tag::ETHEREAL_TAG);
                enumLocks.lock(Items::Tag::TOPUP_TAG);
                enumLocks.lock(Items::Tag::ORBITAL_TAG);
            }
            if (ante < 3) {
                enumLocks.lock(Items::Boss::THE_TOOTH);
                enumLocks.lock(Items::Boss::THE_EYE);
            }
            if (ante < 4) enumLocks.lock(Items::Boss::THE_PLANT);
            if (ante < 5) enumLocks.lock(Items::Boss::THE_SERPENT);
            if (ante < 6) enumLocks.lock(Items::Boss::THE_OX);
            
            // Implement freshProfile and freshRun locks with enums - EXACT same logic as original
            if (freshProfile) {
                // Lock tags
                enumLocks.lock(Items::Tag::NEGATIVE_TAG);
                enumLocks.lock(Items::Tag::FOIL_TAG);
                enumLocks.lock(Items::Tag::HOLOGRAPHIC_TAG);
                enumLocks.lock(Items::Tag::POLYCHROME_TAG);
                enumLocks.lock(Items::Tag::RARE_TAG);
                
                // Lock jokers
                enumLocks.lock(Items::Joker::GOLDEN_TICKET);
                enumLocks.lock(Items::Joker::MR_BONES);
                enumLocks.lock(Items::Joker::ACROBAT);
                enumLocks.lock(Items::Joker::SOCK_AND_BUSKIN);
                enumLocks.lock(Items::Joker::SWASHBUCKLER);
                enumLocks.lock(Items::Joker::TROUBADOUR);
                enumLocks.lock(Items::Joker::CERTIFICATE);
                enumLocks.lock(Items::Joker::SMEARED_JOKER);
                enumLocks.lock(Items::Joker::THROWBACK);
                enumLocks.lock(Items::Joker::HANGING_CHAD);
                enumLocks.lock(Items::Joker::ROUGH_GEM);
                enumLocks.lock(Items::Joker::BLOODSTONE);
                enumLocks.lock(Items::Joker::ARROWHEAD);
                enumLocks.lock(Items::Joker::ONYX_AGATE);
                enumLocks.lock(Items::Joker::GLASS_JOKER);
                enumLocks.lock(Items::Joker::SHOWMAN);
                enumLocks.lock(Items::Joker::FLOWER_POT);
                enumLocks.lock(Items::Joker::BLUEPRINT);
                enumLocks.lock(Items::Joker::WEE_JOKER);
                enumLocks.lock(Items::Joker::MERRY_ANDY);
                enumLocks.lock(Items::Joker::OOPS_ALL_6S);
                enumLocks.lock(Items::Joker::THE_IDOL);
                enumLocks.lock(Items::Joker::SEEING_DOUBLE);
                enumLocks.lock(Items::Joker::MATADOR);
                enumLocks.lock(Items::Joker::HIT_THE_ROAD);
                enumLocks.lock(Items::Joker::THE_DUO);
                enumLocks.lock(Items::Joker::THE_TRIO);
                enumLocks.lock(Items::Joker::THE_FAMILY);
                enumLocks.lock(Items::Joker::THE_ORDER);
                enumLocks.lock(Items::Joker::THE_TRIBE);
                enumLocks.lock(Items::Joker::STUNTMAN);
                enumLocks.lock(Items::Joker::INVISIBLE_JOKER);
                enumLocks.lock(Items::Joker::BRAINSTORM);
                enumLocks.lock(Items::Joker::SATELLITE);
                enumLocks.lock(Items::Joker::SHOOT_THE_MOON);
                enumLocks.lock(Items::Joker::DRIVERS_LICENSE);
                enumLocks.lock(Items::Joker::CARTOMANCER);
                enumLocks.lock(Items::Joker::ASTRONOMER);
                enumLocks.lock(Items::Joker::BURNT_JOKER);
                enumLocks.lock(Items::Joker::BOOTSTRAPS);
                
                // Lock vouchers (already locked above, but keeping for completeness)
                enumLocks.lock(Items::Voucher::OVERSTOCK_PLUS);
                enumLocks.lock(Items::Voucher::LIQUIDATION);
                enumLocks.lock(Items::Voucher::GLOW_UP);
                enumLocks.lock(Items::Voucher::REROLL_GLUT);
                enumLocks.lock(Items::Voucher::OMEN_GLOBE);
                enumLocks.lock(Items::Voucher::OBSERVATORY);
                enumLocks.lock(Items::Voucher::NACHO_TONG);
                enumLocks.lock(Items::Voucher::RECYCLOMANCY);
                enumLocks.lock(Items::Voucher::TAROT_TYCOON);
                enumLocks.lock(Items::Voucher::PLANET_TYCOON);
                enumLocks.lock(Items::Voucher::MONEY_TREE);
                enumLocks.lock(Items::Voucher::ANTIMATTER);
                enumLocks.lock(Items::Voucher::ILLUSION);
                enumLocks.lock(Items::Voucher::PETROGLYPH);
                enumLocks.lock(Items::Voucher::RETCON);
                enumLocks.lock(Items::Voucher::PALETTE);
            }
            
            if (freshRun) {
                // Lock planets
                enumLocks.lock(Items::Planet::PLANET_X);
                enumLocks.lock(Items::Planet::CERES);
                enumLocks.lock(Items::Planet::ERIS);
                
                // Lock jokers (some overlap with freshProfile)
                enumLocks.lock(Items::Joker::STONE_JOKER);
                enumLocks.lock(Items::Joker::STEEL_JOKER);
                enumLocks.lock(Items::Joker::GLASS_JOKER);
                enumLocks.lock(Items::Joker::GOLDEN_TICKET);
                enumLocks.lock(Items::Joker::LUCKY_CAT);
                enumLocks.lock(Items::Joker::CAVENDISH);
                
                // Lock vouchers (same as freshProfile)
                enumLocks.lock(Items::Voucher::OVERSTOCK_PLUS);
                enumLocks.lock(Items::Voucher::LIQUIDATION);
                enumLocks.lock(Items::Voucher::GLOW_UP);
                enumLocks.lock(Items::Voucher::REROLL_GLUT);
                enumLocks.lock(Items::Voucher::OMEN_GLOBE);
                enumLocks.lock(Items::Voucher::OBSERVATORY);
                enumLocks.lock(Items::Voucher::NACHO_TONG);
                enumLocks.lock(Items::Voucher::RECYCLOMANCY);
                enumLocks.lock(Items::Voucher::TAROT_TYCOON);
                enumLocks.lock(Items::Voucher::PLANET_TYCOON);
                enumLocks.lock(Items::Voucher::MONEY_TREE);
                enumLocks.lock(Items::Voucher::ANTIMATTER);
                enumLocks.lock(Items::Voucher::ILLUSION);
                enumLocks.lock(Items::Voucher::PETROGLYPH);
                enumLocks.lock(Items::Voucher::RETCON);
                enumLocks.lock(Items::Voucher::PALETTE);

                enumLocks.lock(Items::PlayedHand::FIVE_OF_A_KIND);
                enumLocks.lock(Items::PlayedHand::FLUSH_FIVE);
                enumLocks.lock(Items::PlayedHand::FLUSH_HOUSE);
            }
        }

        void initUnlocks(int ante, bool freshProfile) {
            if (ante == 2) {
                enumLocks.unlock(Items::Boss::THE_MOUTH);
                enumLocks.unlock(Items::Boss::THE_FISH);
                enumLocks.unlock(Items::Boss::THE_WALL);
                enumLocks.unlock(Items::Boss::THE_HOUSE);
                enumLocks.unlock(Items::Boss::THE_MARK);
                enumLocks.unlock(Items::Boss::THE_WHEEL);
                enumLocks.unlock(Items::Boss::THE_ARM);
                enumLocks.unlock(Items::Boss::THE_WATER);
                enumLocks.unlock(Items::Boss::THE_NEEDLE);
                enumLocks.unlock(Items::Boss::THE_FLINT);
                if (!freshProfile) enumLocks.unlock(Items::Tag::NEGATIVE_TAG);
                enumLocks.unlock(Items::Tag::STANDARD_TAG);
                enumLocks.unlock(Items::Tag::METEOR_TAG);
                enumLocks.unlock(Items::Tag::BUFFOON_TAG);
                enumLocks.unlock(Items::Tag::HANDY_TAG);
                enumLocks.unlock(Items::Tag::GARBAGE_TAG);
                enumLocks.unlock(Items::Tag::ETHEREAL_TAG);
                enumLocks.unlock(Items::Tag::TOPUP_TAG);
                enumLocks.unlock(Items::Tag::ORBITAL_TAG);
            }
            if (ante == 3) {
                enumLocks.unlock(Items::Boss::THE_TOOTH);
                enumLocks.unlock(Items::Boss::THE_EYE);
            }
            if (ante == 4) enumLocks.unlock(Items::Boss::THE_PLANT);
            if (ante == 5) enumLocks.unlock(Items::Boss::THE_SERPENT);
            if (ante == 6) enumLocks.unlock(Items::Boss::THE_OX);
        }

        
        // ========================================
        // UTILITY METHODS
        // ========================================
        
        void setDeck(const std::string& d) { deck = d; }
        void setStake(const std::string& s) { stake = s; }
        void setJokerUnlocked(Items::Joker joker, bool unlocked) {
            if (unlocked) enumLocks.unlock(joker);
            else enumLocks.lock(joker);
        }
        
        // Voucher activation and unlocking
        void activateVoucher_enum(Items::Voucher voucher) {
            enumLocks.lock(voucher);
            // enumLocks.activate(voucher);
            
            // EXACT SAME LOGIC as original: Unlock next tier voucher based on VOUCHERS array pairing
            // Voucher pairs in order: [base, upgrade, base, upgrade, ...]
            // We need to find which pair this voucher belongs to and unlock its partner
            
            // Convert ALL_VOUCHERS to pairs and find the matching unlock
            for (size_t i = 0; i < Items::ALL_VOUCHERS.size(); i += 2) {
                if (Items::ALL_VOUCHERS[i] == voucher && i + 1 < Items::ALL_VOUCHERS.size()) {
                    enumLocks.unlock(Items::ALL_VOUCHERS[i + 1]);
                    break;
                }
            }
        }
        
        const std::string& getSeed() const { return seed; }
        bool isShowman() const { return showman; }
        long getVersion() const { return version; }
    };

} // namespace Items
