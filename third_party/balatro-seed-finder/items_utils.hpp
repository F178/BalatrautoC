#pragma once

#include "items.hpp"
#include "rand_util.hpp"
#include <bitset>
#include <array>

// High-performance enum utilities for critical hot paths
// Focused on maximum speed with minimal overhead

namespace Locks {
    
    // ========================================
    // LOCK CHECKING
    // ========================================

    class EnumLockSystem {
    private:
        std::bitset<static_cast<size_t>(Items::Joker::COUNT)> joker_locks;
        std::bitset<static_cast<size_t>(Items::Tarot::COUNT)> tarot_locks;
        std::bitset<static_cast<size_t>(Items::Planet::COUNT)> planet_locks;
        std::bitset<static_cast<size_t>(Items::Spectral::COUNT)> spectral_locks;
        std::bitset<static_cast<size_t>(Items::Tag::COUNT)> tag_locks;
        std::bitset<static_cast<size_t>(Items::Boss::COUNT)> boss_locks;
        std::bitset<static_cast<size_t>(Items::Voucher::COUNT)> voucher_locks;
        
        std::bitset<static_cast<size_t>(Items::PlayedHand::COUNT)> played_hands;

        std::bitset<static_cast<size_t>(Items::Voucher::COUNT)> vouchers_active;
        
    public:
        inline bool isLocked(Items::Joker j) const {
            return joker_locks[static_cast<size_t>(j)];
        }
        
        inline bool isLocked(Items::Tarot t) const {
            return tarot_locks[static_cast<size_t>(t)];
        }
        
        inline bool isLocked(Items::Planet p) const {
            return planet_locks[static_cast<size_t>(p)];
        }
        
        inline bool isLocked(Items::Spectral s) const {
            return spectral_locks[static_cast<size_t>(s)];
        }
        
        inline bool isLocked(Items::Tag tag) const {
            return tag_locks[static_cast<size_t>(tag)];
        }
        
        inline bool isLocked(Items::Boss b) const {
            return boss_locks[static_cast<size_t>(b)];
        }
        
        inline bool isLocked(Items::Voucher v) const {
            return voucher_locks[static_cast<size_t>(v)];
        }

        inline bool isVoucherActive(Items::Voucher v) const {
            return vouchers_active[static_cast<size_t>(v)];
        }

        inline bool isLocked(Items::PlayedHand v) const {
            return played_hands[static_cast<size_t>(v)];
        }
        
        // lock/unlock operations
        inline void lock(Items::Joker j) { joker_locks[static_cast<size_t>(j)] = true; }
        inline void unlock(Items::Joker j) { joker_locks[static_cast<size_t>(j)] = false; }
        
        inline void lock(Items::Tarot t) { tarot_locks[static_cast<size_t>(t)] = true; }
        inline void unlock(Items::Tarot t) { tarot_locks[static_cast<size_t>(t)] = false; }
        
        inline void lock(Items::Planet p) { planet_locks[static_cast<size_t>(p)] = true; }
        inline void unlock(Items::Planet p) { planet_locks[static_cast<size_t>(p)] = false; }
        
        inline void lock(Items::Spectral s) { spectral_locks[static_cast<size_t>(s)] = true; }
        inline void unlock(Items::Spectral s) { spectral_locks[static_cast<size_t>(s)] = false; }
        
        inline void lock(Items::Tag tag) { tag_locks[static_cast<size_t>(tag)] = true; }
        inline void unlock(Items::Tag tag) { tag_locks[static_cast<size_t>(tag)] = false; }
        
        inline void lock(Items::Boss b) { boss_locks[static_cast<size_t>(b)] = true; }
        inline void unlock(Items::Boss b) { boss_locks[static_cast<size_t>(b)] = false; }
        
        inline void lock(Items::Voucher v) { voucher_locks[static_cast<size_t>(v)] = true; }
        inline void unlock(Items::Voucher v) { voucher_locks[static_cast<size_t>(v)] = false; }

        inline void activate(Items::Voucher v) { vouchers_active[static_cast<size_t>(v)] = true; }

        inline void lock(Items::PlayedHand v) { played_hands[static_cast<size_t>(v)] = true; }
        inline void unlock(Items::PlayedHand v) { played_hands[static_cast<size_t>(v)] = false; }

        
        // Bulk operations for initialization
        void resetAll() {
            joker_locks.reset();
            tarot_locks.reset();
            planet_locks.reset();
            spectral_locks.reset();
            tag_locks.reset();
            boss_locks.reset();
            voucher_locks.reset();
            vouchers_active.reset();
        }
    };

    // ========================================
    // SPECIALIZED JOKER DATA STRUCTURES
    // ========================================
    

    // ========================================
    // ENUM-BASED CARD STRUCTURE
    // ========================================
    
    // ========================================
    // RARITY-BASED FAST LOOKUP
    // ========================================
    
    // Get joker rarity without string comparisons
    constexpr uint8_t getJokerRarity(Items::Joker joker) {
        auto j = static_cast<uint16_t>(joker);
        if (j <= static_cast<uint16_t>(Items::Joker::COMMON_END)) return 1;
        if (j <= static_cast<uint16_t>(Items::Joker::UNCOMMON_END)) return 2;
        if (j <= static_cast<uint16_t>(Items::Joker::RARE_END)) return 3;
        if (j <= static_cast<uint16_t>(Items::Joker::LEGENDARY_END)) return 4;
        return 0; // Invalid
    }

    
} // namespace Locks