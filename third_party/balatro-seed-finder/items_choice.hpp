#pragma once

#include "items.hpp"
#include "items_utils.hpp"

#include <functional>

namespace Items {

    struct OptimizedJokerData {
        Items::Joker joker;
        uint8_t rarity;  // 1=common, 2=uncommon, 3=rare, 4=legendary
        Items::Edition edition;
        bool eternal : 1;
        bool perishable : 1;
        bool rental : 1;
        bool padding : 5;  // Pad to byte boundary

        OptimizedJokerData()
            : joker(Items::Joker::JOKER), rarity(1), edition(Items::Edition::NO_EDITION)
            , eternal(false), perishable(false), rental(false), padding(false) {}

        OptimizedJokerData(Items::Joker j, uint8_t r, Items::Edition e, bool et = false, bool p = false, bool rent = false)
            : joker(j), rarity(r), edition(e), eternal(et), perishable(p), rental(rent), padding(false) {}
    };

    struct CardEnum {
        std::string base;       // Keep as string for now (CARDS array is complex)
        Items::Enhancement enhancement;
        Items::Edition edition;
        Items::Seal seal;

        CardEnum()
            : base("C_A"), enhancement(Items::Enhancement::NO_ENHANCEMENT)
            , edition(Items::Edition::NO_EDITION), seal(Items::Seal::NO_SEAL) {}

        CardEnum(const std::string& b, Items::Enhancement enh, Items::Edition ed, Items::Seal s)
            : base(b), enhancement(enh), edition(ed), seal(s) {}
    };

    struct OptimizedShopItem {
        enum class Type : uint8_t { JOKER = 0, TAROT = 1, PLANET = 2, SPECTRAL = 3, PLAYING_CARD = 4 };

        Type type;
        union {
            Items::Joker joker;
            Items::Tarot tarot;
            Items::Planet planet;
            Items::Spectral spectral;
            uint16_t raw_value;  // For generic access
        } item{};
        OptimizedJokerData joker_data;  // Only valid when type == JOKER

        OptimizedShopItem() : type(Type::TAROT) { item.tarot = Items::Tarot::THE_FOOL; }

        explicit OptimizedShopItem(Items::Tarot t) : type(Type::TAROT) { item.tarot = t; }
        explicit OptimizedShopItem(Items::Planet p) : type(Type::PLANET) { item.planet = p; }
        explicit OptimizedShopItem(Items::Spectral s) : type(Type::SPECTRAL) { item.spectral = s; }

        OptimizedShopItem(Items::Joker j, const OptimizedJokerData& data)
            : type(Type::JOKER), joker_data(data) { item.joker = j; }
    };


    // Forward declare get_node_func type to match Instance::get_node
    using GetNodeFunc = std::function<double(const std::string&)>;

    // TODO : Check if this is still necessary
    template<typename EnumType, size_t ArraySize>
    class FastRandChoice {
    private:
        const std::array<EnumType, ArraySize>& items;

    public:
        constexpr explicit FastRandChoice(const std::array<EnumType, ArraySize>& item_array)
            : items(item_array) {}

        EnumType operator()(LuaRandom& rng, const Locks::EnumLockSystem& locks, bool showman = false) const {
            EnumType item = items[rng.randint(0, ArraySize - 1)];

            if (!showman && locks.isLocked(item)) {
                int resample = 2;
                while (true) {
                    LuaRandom resample_rng(rng.random()); // Use current RNG to seed new one
                    item = items[resample_rng.randint(0, ArraySize - 1)];
                    resample++;

                    if (!locks.isLocked(item) || resample > 1000) {
                        return item;
                    }
                }
            }
            return item;
        }

        EnumType simple(LuaRandom& rng) const {
            return items[rng.randint(0, ArraySize - 1)];
        }
    };

    // Enum-based randchoice with exact same behavior as string version
    template<typename EnumType, size_t ArraySize>
    EnumType enum_randchoice(const std::string& ID, const std::array<EnumType, ArraySize>& items,
                           const Locks::EnumLockSystem& locks, bool showman, const GetNodeFunc& get_node) {
        // EXACT SAME LOGIC as original randchoice:
        LuaRandom rng(get_node(ID));
        EnumType item = items[rng.randint(0, items.size()-1)];

        // Check if locked or invalid (RETRY equivalent)
        bool isRetry = (item == static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(EnumType::INVALID)));
        if ((showman == false && locks.isLocked(item)) || isRetry) {
            int resample = 2;
            while (true) {
                rng = LuaRandom(get_node(ID + "_resample" + std::to_string(resample)));
                item = items[rng.randint(0, items.size()-1)];
                resample++;
                bool isNotRetry = (item != static_cast<EnumType>(static_cast<std::underlying_type_t<EnumType>>(EnumType::INVALID)));
                if ((isNotRetry && !locks.isLocked(item)) || resample > 1000) return item;
            }
        }
        return item;
    }

    inline Joker CommonJokerChoice(const GetNodeFunc &get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "joker") {
        return enum_randchoice(ID, COMMON_JOKERS, locks, showman, get_node);
    }
    
    inline Joker UncommonJokerChoice(const GetNodeFunc &get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "joker") {
        return enum_randchoice(ID, UNCOMMON_JOKERS, locks, showman, get_node);
    }
    
    inline Joker RareJokerChoice(const GetNodeFunc& get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "joker") {
        return enum_randchoice(ID, RARE_JOKERS, locks, showman, get_node);
    }
    
    inline Joker LegendaryJokerChoice(const GetNodeFunc& get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "joker") {
        return enum_randchoice(ID, LEGENDARY_JOKERS, locks, showman, get_node);
    }
    
    inline Tarot TarotChoice(const GetNodeFunc& get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "tarot") {
        return enum_randchoice(ID, ALL_TAROTS, locks, showman, get_node);
    }
    
    inline Planet PlanetChoice(const GetNodeFunc& get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "planet") {
        return enum_randchoice(ID, ALL_PLANETS, locks, showman, get_node);
    }
    
    inline Spectral SpectralChoice(const GetNodeFunc& get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "spectral") {
        return enum_randchoice(ID, ALL_SPECTRALS, locks, showman, get_node);
    }
    
    inline Tag TagChoice(const GetNodeFunc& get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "tag") {
        return enum_randchoice(ID, ALL_TAGS, locks, showman, get_node);
    }
    
    inline Boss BossChoice(const GetNodeFunc& get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "boss") {
        return enum_randchoice(ID, ALL_BOSSES, locks, showman, get_node);
    }
    
    inline Voucher VoucherChoice(const GetNodeFunc& get_node, const Locks::EnumLockSystem& locks, bool showman, const std::string& ID = "voucher") {
        return enum_randchoice(ID, ALL_VOUCHERS, locks, showman, get_node);
    }

    inline Enhancement EnhancementChoice(const GetNodeFunc& get_node, const std::string& ID = "enhancement") {
        LuaRandom rng(get_node(ID));
        return ALL_ENHANCEMENTS[rng.randint(0, ALL_ENHANCEMENTS.size() - 1)];
    }

    inline Pack PackChoice(const GetNodeFunc& get_node, const std::string& ID = "pack") {
        LuaRandom rng(get_node(ID));
        double poll = rng.random() * ALL_PACKS[0].weight;
        size_t idx = 1;
        double weight = 0;

        while (weight < poll && idx < ALL_PACKS.size()) {
            weight += ALL_PACKS[idx].weight;
            idx++;
        }

        return ALL_PACKS[idx - 1].pack;
    }
}
