#include <limits>

#include "Utils.h"
#include "Settings.h"

#include "ClibUtil/editorID.hpp"

namespace Utils {

    std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    static bool MatchesAnyPattern(const std::string& lower, const std::vector<std::string>& patterns) {
        for (const auto& pat : patterns) {
            if (pat == "ice") {
                size_t pos = lower.find("ice");
                bool foundValidIce = false;

                while (pos != std::string::npos) {
                    // valid if NOT part of "voice"
                    if (!(pos >= 2 && lower[pos - 2] == 'v' && lower[pos - 1] == 'o')) {
                        foundValidIce = true;
                        break;
                    }
                    pos = lower.find("ice", pos + 1);
                }

                if (foundValidIce) return true;

            } else if (lower.find(pat) != std::string::npos) {
                return true;
            }
        }

        return false;
    }

    ElementType GetProjectileType(RE::Projectile* proj) {
        ElementType ret = GetProjectileTypeNoLogs(proj);
        return ret;
        std::string typeStr = "";

        if (ret == ElementType::Cold)
            typeStr = "Cold";
        else if (ret == ElementType::Fire)
            typeStr = "Fire";
        else if (ret == ElementType::Water)
            typeStr = "Water";
        else
            typeStr = "Neutral";
        logger::info("Projectile Type Detection: {} -> {}", proj ? proj->GetFormID() : 0, typeStr);
        return ret;
    }

    ElementType GetProjectileTypeNoLogs(RE::Projectile* proj) {
        static auto* set = Settings::GetSingleton();
        static auto* DetectionCache = DetectionCache::GetSingleton();

        if (!proj) return ElementType::Neutral;

        auto* base = proj->GetBaseObject();
        if (!base) return ElementType::Neutral;

        auto baseFormID = base->GetFormID();

        // 1. Check cache by base object
        auto cacheIt = DetectionCache->Get(baseFormID);
        if (cacheIt.has_value()) {
            return cacheIt.value();
        }

        ElementType type = ElementType::Neutral;

        // Helper lambda for string pattern detection
        auto CheckPatterns = [&](RE::TESForm* form) -> ElementType {
            if (!form) return ElementType::Neutral;

            auto TryMatch = [&](const std::string& str) -> ElementType {
                std::string lowerStr = ToLower(str);
                if (MatchesAnyPattern(lowerStr, set->neutralSources)) return ElementType::Neutral;
                if (MatchesAnyPattern(lowerStr, set->coldSources)) return ElementType::Cold;
                if (MatchesAnyPattern(lowerStr, set->fireSources)) return ElementType::Fire;
                if (MatchesAnyPattern(lowerStr, set->waterSources)) return ElementType::Water;
                return ElementType::Neutral;
            };

            if (auto editorID = form->GetFormEditorID()) {
                type = TryMatch(editorID);
                if (type != ElementType::Neutral) return type;
            }
            auto editorID = clib_util::editorID::get_editorID(form);
            if (!editorID.empty()) {
                type = TryMatch(editorID);
                if (type != ElementType::Neutral) return type;
            }
            if (auto fullName = form->GetName()) {
                type = TryMatch(fullName);
                if (type != ElementType::Neutral) return type;
            }

            return ElementType::Neutral;
        };

        // 3. Check spell source
        if (auto spellSource = proj->GetProjectileRuntimeData().spell) {
            type = CheckPatterns(spellSource);
            if (type != ElementType::Neutral) {
                DetectionCache->Add(baseFormID, type);
                return type;
            }

            // Check base effects
            for (auto magEf : spellSource->effects) {
                if (magEf && magEf->baseEffect) {
                    type = CheckPatterns(magEf->baseEffect);
                    if (type != ElementType::Neutral) {
                        DetectionCache->Add(baseFormID, type);
                        return type;
                    }
                }
            }
        }

        // 4. Ammo source
        type = CheckPatterns(proj->GetProjectileRuntimeData().ammoSource);
        if (type != ElementType::Neutral) {
            DetectionCache->Add(baseFormID, type);
            return type;
        }

        // 5. Weapon source
        type = CheckPatterns(proj->GetProjectileRuntimeData().weaponSource);
        if (type != ElementType::Neutral) {
            DetectionCache->Add(baseFormID, type);
            return type;
        }

        // 6. Projectile's own base object
        type = CheckPatterns(base);
        DetectionCache->Add(baseFormID, type);
        return type;
    }

    ElementType GetExplosionType(RE::Explosion* exp) {
        static auto* set = Settings::GetSingleton();
        static auto* DetectionCache = DetectionCache::GetSingleton();

        if (!exp) return ElementType::Neutral;

        auto* base = exp->GetBaseObject();
        if (!base) return ElementType::Neutral;

        auto baseFormID = base->GetFormID();

        // 1. Cache check
        auto cacheIt = DetectionCache->Get(baseFormID);
        if (cacheIt.has_value()) {
            return cacheIt.value();
        }

        ElementType type = ElementType::Neutral;

        // Helper lambda for pattern matching
        auto CheckPatterns = [&](RE::TESForm* form) -> ElementType {
            if (!form) return ElementType::Neutral;

            auto TryMatch = [&](const std::string& str) -> ElementType {
                std::string lowerStr = ToLower(str);
                if (MatchesAnyPattern(lowerStr, set->neutralSources)) return ElementType::Neutral;
                if (MatchesAnyPattern(lowerStr, set->coldSources)) return ElementType::Cold;
                if (MatchesAnyPattern(lowerStr, set->fireSources)) return ElementType::Fire;
                if (MatchesAnyPattern(lowerStr, set->waterSources)) return ElementType::Water;
                return ElementType::Neutral;
            };

            if (auto editorID = form->GetFormEditorID()) {
                type = TryMatch(editorID);
                if (type != ElementType::Neutral) return type;
            }
            auto editorID = clib_util::editorID::get_editorID(form);
            if (!editorID.empty()) {
                type = TryMatch(editorID);
                if (type != ElementType::Neutral) return type;
            }
            if (auto fullName = form->GetName()) {
                type = TryMatch(fullName);
                if (type != ElementType::Neutral) return type;
            }
            if (auto model = form->As<RE::TESModel>()) {
                type = TryMatch(model->model.c_str());
                if (type != ElementType::Neutral) return type;
            }

            return ElementType::Neutral;
        };

        type = CheckPatterns(base);
        DetectionCache->Add(baseFormID, type);
        return type;
    }

    float GetDamageFromProjectile(RE::Projectile* proj) {
        if (!proj) {
            return 0.0f;
        }

        float ret = 0.0f;

        const auto& data = proj->GetProjectileRuntimeData();

        if (data.weaponSource) {
            if (data.weaponSource->attackDamage > 0.0f) {
                ret = static_cast<float>(data.weaponSource->attackDamage);
            }
        }

        if (data.ammoSource) {
            auto ammoRuntimeData = data.ammoSource->GetRuntimeData().data;
            if (ammoRuntimeData.damage > 0.0f) {
                ret = static_cast<float>(ammoRuntimeData.damage);
            }
        }

        if (data.spell) {
            float spellDamage = 0.0f;
            for (auto& effect : data.spell->effects) {
                if (effect) {
                    bool hostile = effect->IsHostile();
                    bool restoration = effect->baseEffect->GetMagickSkill() == RE::ActorValue::kRestoration;
                    bool illusion = effect->baseEffect->GetMagickSkill() == RE::ActorValue::kIllusion;

                    auto effectArchetype = effect->baseEffect->GetArchetype();
                    bool archetype = effectArchetype == RE::EffectArchetypes::ArchetypeID::kValueModifier ||
                                     effectArchetype == RE::EffectArchetypes::ArchetypeID::kDualValueModifier;
                    RE::TESObjectREFR* shooter = data.shooter.get().get();
                    RE::TESObjectREFR* target = data.desiredTarget.get().get();
                    bool conditionsTrue = effect->conditions.IsTrue(shooter, target) &&
                                          effect->baseEffect->conditions.IsTrue(shooter, target);
                    if (hostile && !restoration && !illusion && archetype && conditionsTrue) {
                        spellDamage += effect->GetMagnitude();
                    }
                }
            }
            ret = spellDamage;
        }
        return ret;
    }

    // Based on po3's Splashes-of-Skyrim
    // https://github.com/powerof3/Splashes-of-Skyrim
    float get_water_height(const RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_pos) {
        float waterHeight = a_ref->GetWaterHeight();

        if (!(waterHeight == -RE::NI_INFINITY)) {
            return waterHeight;
        }

        if (const auto waterManager = RE::TESWaterSystem::GetSingleton()) {
            const RE::BSSpinLockGuard locker(waterManager->lock);  // serialize against water-system mutations

            const auto settings = Settings::GetSingleton();

            const auto get_nearest_water_object_height = [&]() {
                for (const auto& waterObjectPtr : waterManager->waterObjects) {
                    const auto waterObject = waterObjectPtr.get();  // read slot once to avoid data race
                    if (!waterObject) {
                        continue;
                    }
                    for (const auto& boundPtr : waterObject->multiBounds) {
                        const auto bound = boundPtr.get();
                        if (!bound) {
                            continue;
                        }
                        if (auto size{bound->size}; size.z <= 10.0f) {  // avoid sloped water
                            auto center{bound->center};
                            const auto boundMin = center - size;
                            const auto boundMax = center + size;
                            if (!(a_pos.x < boundMin.x || a_pos.x > boundMax.x || a_pos.y < boundMin.y ||
                                  a_pos.y > boundMax.y)) {
                                return center.z;
                            }
                        }
                    }
                }

                return -RE::NI_INFINITY;
            };

            waterHeight = get_nearest_water_object_height();
        }

        return waterHeight;
    }

    std::pair<float, float> get_explosion_water_radius(const RE::Explosion* a_explosion, float base_radius) {
        auto pos = a_explosion->GetPosition();
        auto waterHeight = Utils::get_water_height(a_explosion, pos);

        if (waterHeight == -RE::NI_INFINITY) {
            return std::make_pair(0.0f, waterHeight);
        }

        float explosionZ = pos.z;

        if (explosionZ < waterHeight) {
            return std::make_pair(base_radius, waterHeight);
        }

        float heightDiff = explosionZ - waterHeight;

        if (heightDiff >= base_radius) {
            return std::make_pair(0.0f, waterHeight);
        }

        float waterAffectedRadius = base_radius - heightDiff;

        return std::make_pair(std::max(waterAffectedRadius, 0.0f), waterHeight);
    }
}  // namespace Utils

