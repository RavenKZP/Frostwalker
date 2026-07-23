#pragma once

#include "REX/REX.h"
#include "CLibUtilsQTR/Tasker.hpp"

#include <shared_mutex>

enum ElementType {
    Neutral = 0,
    Fire,
    Shock,
    Cold,
    Water,
};

class DetectionCache : public REX::Singleton<DetectionCache> {
public:
    std::optional<ElementType> Get(RE::FormID id) {
        std::shared_lock<std::shared_mutex> readLock(cacheMutex_);
        auto it = cache_.find(id);
        if (it != cache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void Add(RE::FormID id, ElementType type) {
        std::unique_lock<std::shared_mutex> writeLock(cacheMutex_);
        cache_[id] = type;

        /*
        if (cacheMutex_.try_lock()) {  // try to grab exclusive lock quickly
            cache_[id] = type;
        } else {
            clib_utilsQTR::Tasker::GetSingleton()->PushTask(
                [this, id, type]() {
                    std::unique_lock<std::shared_mutex> writeLock(cacheMutex_);
                    cache_[id] = type;
                },
                0);
        }
        */
    }

private:
    std::unordered_map<RE::FormID, ElementType> cache_;
    mutable std::shared_mutex cacheMutex_;
};

namespace Utils {

    std::string ToLower(std::string s);

    ElementType GetProjectileType(RE::Projectile* proj);
    ElementType GetProjectileTypeNoLogs(RE::Projectile* proj);
    ElementType GetExplosionType(RE::Explosion* exp);

    float GetDamageFromProjectile(RE::Projectile* proj);

    // Returns: {water_height, isLava}
    std::pair<float, bool> get_water_height(const RE::NiPoint3& a_pos);

    // Returns: {actual_radius, water_height, isLava}
    std::tuple<float, float, bool> get_explosion_water_radius(const RE::Explosion* a_explosion, float base_radius);
}