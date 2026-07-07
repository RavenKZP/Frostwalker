#pragma once

enum ElementType {
    Neutral = 0,
    Fire,
    Shock,
    Cold,
    Water,
};

namespace Utils {

    std::string ToLower(std::string s);

    ElementType GetProjectileType(RE::Projectile* proj);
    ElementType GetProjectileTypeNoLogs(RE::Projectile* proj);
    ElementType GetExplosionType(RE::Explosion* exp);

    float GetDamageFromProjectile(RE::Projectile* proj);

    float get_water_height(const RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_pos);

    // Returns: {actual_radius, water_height}
    std::pair<float, float> get_explosion_water_radius(const RE::Explosion* a_explosion, float base_radius);
}