#pragma once

#include <random>
#include <chrono>
#include <algorithm>

#include "REX/REX.h"
#include "Settings.h"
#include "Utils.h"

#include "ClibUtil/editorID.hpp"
// #include "CLibUtilsQTR/DrawDebug.hpp"

inline constexpr float M_PI_F = 3.14159265358979323846f;

enum TYPE : std::uint32_t {
    kMissile = 0,
    kFlame,
    kCone,
    kArrow,
    kBeam,
    kExplosion,
};

enum class IceSize { Chunk = 1, Small, Medium, Large };

struct IceHazardData {
    RE::ObjectRefHandle handle;
    IceSize size;
};

namespace Frostwalker {
    class Manager : public REX::Singleton<Manager> {
    public:
        Manager() {
            // ctor
        };

        void OnGameLoad() {
            HazardIceChunks.clear();
            lastProcessTime.clear();

            RE::TES::GetSingleton()->ForEachReference([this](RE::TESObjectREFR* a_ref) {
                if (a_ref && a_ref->IsDynamicForm()) {
                    if (auto hazardType = a_ref->As<RE::Hazard>()) {
                        std::string filename = "";
                        if (auto base_obj = a_ref->GetBaseObject()) {
                            if (auto ModFile = base_obj->GetFile()) {
                                filename = ModFile->GetFilename();
                            }
                        }
                        if (filename == "Frostwalker.esp") {
                            // Assume Chunk it's more optimal
                            HazardIceChunks[hazardType->GetFormID()] = {a_ref->GetHandle(), IceSize::Chunk};
                        }
                    }
                }
                return RE::BSContainer::ForEachResult::kContinue;
            });
        }

        static float RandomFloat(float min, float max) {
            static thread_local std::mt19937 rng{std::random_device{}()};
            std::uniform_real_distribution<float> dist(min, max);
            return dist(rng);
        }

        static int RandomInt(int min, int max) {
            static thread_local std::mt19937 rng{std::random_device{}()};
            std::uniform_int_distribution<int> dist(min, max);
            return dist(rng);
        }

        static void MeltObject(RE::TESObjectREFR* ref, float lifetime, float age) {
            auto* set = Settings::GetSingleton();
            float remainingLife = lifetime - age;

            if (remainingLife <= set->meltingTime && remainingLife > 0.0f) {
                if (auto obj3d = ref->Get3D()) {
                    float meltProgress = std::max(0.0f, remainingLife / set->meltingTime);
                    obj3d->local.rotate.entry[2][2] = meltProgress;
                    if (auto fade = obj3d->AsFadeNode()) {
                        fade->GetRuntimeData().currentFade = meltProgress;
                    }
                    RE::NiUpdateData updData;
                    updData.flags = RE::NiUpdateData::Flag::kNone;
                    updData.time = 0.0f;
                    obj3d->UpdateTransformAndBounds(updData);
                }
            } else if (remainingLife <= 0.0f) {
                if (auto obj3d = ref->Get3D()) {
                    obj3d->local.rotate.entry[2][2] = 0.0f;
                    if (auto fade = obj3d->AsFadeNode()) {
                        fade->GetRuntimeData().currentFade = 0.0f;
                    }
                }
            } else {
                if (auto obj3d = ref->Get3D()) {
                    if (obj3d->local.rotate.entry[2][2] < 1.0f) {
                        obj3d->local.rotate.entry[2][2] = 1.0f;
                        if (auto fade = obj3d->AsFadeNode()) {
                            fade->GetRuntimeData().currentFade = 1.0f;
                        }
                        RE::NiUpdateData updData;
                        updData.flags = RE::NiUpdateData::Flag::kNone;
                        updData.time = 0.0f;
                        obj3d->UpdateTransformAndBounds(updData);
                    }
                }
            }
        }

        bool ShouldFreezeWater(RE::Actor* actor) {
            if (!actor) return false;

            auto* set = Settings::GetSingleton();
            // Frostwalker creatures
            if (actor->GetActorBase()->HasKeyword(set->Frostwalker_FrostWalk_Keywords[0])) {
                return true;
            }

            for (auto FW_Keyword : set->Frostwalker_FrostWalk_Keywords) {
                if (actor->HasMagicEffectWithKeyword(FW_Keyword)) {
                    return true;
                }
            }
            return false;
        }

        bool ShouldHardRockLava(RE::Actor* actor) {
            if (!actor) return false;

            auto* set = Settings::GetSingleton();
            // Frostwalker creatures
            if (actor->GetActorBase()->HasKeyword(set->Frostwalker_LavaWalk_Keywords[0])) {
                return true;
            }

            for (auto FW_Keyword : set->Frostwalker_LavaWalk_Keywords) {
                if (actor->HasMagicEffectWithKeyword(FW_Keyword)) {
                    return true;
                }
            }
            return false;
        }

        bool ShouldMeltIce(RE::Actor* actor) {
            if (!actor) return false;

            auto* set = Settings::GetSingleton();
            // Frostwalker creatures
            if (actor->GetActorBase()->HasKeyword(set->Frostwalker_FireWalk_Keywords[0]) ||
                actor->HasKeyword(set->Frostwalker_FireWalk_Keywords[0])) {
                return true;
            }

            for (auto FW_Keyword : set->Frostwalker_FireWalk_Keywords) {
                if (actor->HasMagicEffectWithKeyword(FW_Keyword)) {
                    return true;
                }
            }
            return false;
        }

        void FrostWalk(RE::Actor* a_actor) {
            if (!a_actor || a_actor->IsDead() || a_actor->IsDisabled() || a_actor->IsDeleted() ||
                !a_actor->Is3DLoaded()) {
                return;
            }

            auto actorPos = a_actor->GetPosition();
            auto waterHeight = Utils::get_water_height(a_actor, actorPos);
            auto actorHeight = a_actor->GetHeight();
            if (actorHeight <= 0.0f) {
                actorHeight = 50.0f;  // Default height if not set
            }
            auto spawnPos = actorPos;

            if (!(waterHeight == -RE::NI_INFINITY)) {
                const auto level = (waterHeight + 100 - actorPos.z) / actorHeight;
                auto* set = Settings::GetSingleton();

                bool isLava = isWaterLava(a_actor->GetParentCell(), waterHeight);

                if (level >= 0.1f && (ShouldFreezeWater(a_actor)  || (isLava && ShouldHardRockLava(a_actor )))) {
                    if (actorPos.z < waterHeight - 50) {
                        // Actor is too deep in water, don't spawn ice chunk
                        return;
                    }
                    spawnPos.z = waterHeight;

                    for (auto& hazard : HazardIceChunks) {
                        if (auto hazardRef = hazard.second.handle.get().get()) {
                            auto HazardPos = hazardRef->GetPosition();
                            float distance = spawnPos.GetDistance(HazardPos);
                            if (distance < set->IceDistance) {
                                if (auto hazardHazard = hazardRef->As<RE::Hazard>()) {
                                    hazardHazard->GetHazardRuntimeData().age = 0.0f;
                                }
                                return;
                            }
                        }
                    }
                    
                    spawnIceChunk(50, spawnPos, isLava, true);

                } else if (ShouldMeltIce(a_actor)) {
                    for (auto& hazard : HazardIceChunks) {
                        if (auto hazardRef = hazard.second.handle.get().get()) {
                            auto HazardPos = hazardRef->GetPosition();

                            float distanceX = std::abs(actorPos.x - HazardPos.x);
                            float distanceY = std::abs(actorPos.y - HazardPos.y);
                            float distanceZ = std::abs(actorPos.z - HazardPos.z);

                            auto hazardSize = hazardRef->GetBoundMax() - hazardRef->GetBoundMin();
                            if (distanceX < hazardSize.x && distanceY < hazardSize.y &&
                                distanceZ < hazardSize.z + actorHeight) {
                                if (auto hazardHazard = hazardRef->As<RE::Hazard>()) {
                                    float threshold = hazardHazard->GetHazardRuntimeData().lifetime - (set->meltingTime*3);
                                    if (hazardHazard->GetHazardRuntimeData().age < threshold) {
                                        hazardHazard->GetHazardRuntimeData().age = threshold;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        void FrameUpdate(float a_delta) {
            // auto start = std::chrono::high_resolution_clock::now();
            auto set = Settings::GetSingleton();
            if (!set->ModActive) {
                return;
            }
            //DebugAPI_IMPL::DebugAPI::GetSingleton()->Update();

            auto HazardsToRemove = std::vector<RE::FormID>{};

            for (auto HazardHandle : HazardIceChunks) {
                if (auto hazardRef = HazardHandle.second.handle.get().get()) {
                    auto hazardType = hazardRef->As<RE::Hazard>();
                    // logger::debug("Hazard IceChunk with and lifetime={} and age={}",
                    //               hazardType->GetHazardRuntimeData().lifetime,
                    //               hazardType->GetHazardRuntimeData().age);

                    // If lifetime is close to end, start "Melting" the ice chunk by reducing its scale, z Scale and
                    // visability
                    float lifetime = hazardType->GetHazardRuntimeData().lifetime;
                    float age = hazardType->GetHazardRuntimeData().age;
                    float remainingLife = lifetime - age;
                    if (remainingLife <= set->meltingTime && remainingLife > 0.0f) {
                        MeltObject(hazardRef, lifetime, age);
                    }

                } else {
                    // logger::debug("Hazard ref {} is no longer valid, removing from HazardIceChunks",
                    //               HazardHandle.first);
                    HazardsToRemove.push_back(HazardHandle.first);
                }
            }

            for (auto hazard : HazardsToRemove) {
                HazardIceChunks.erase(hazard);
            }

            // auto end = std::chrono::high_resolution_clock::now();
            // std::chrono::duration<double, std::milli> elapsed = end - start;
            // logger::debug("FrameUpdate took {} ms", elapsed.count());
        }

        // Based on po3's Splashes-of-Skyrim
        // https://github.com/powerof3/Splashes-of-Skyrim
        void ProjectileUpdate(RE::Projectile* a_projectile, TYPE type) {
            if (!a_projectile) {
                return;
            }
            /*
            std::string typeStr;
            switch (type) {
                case kMissile:
                    typeStr = "Missile";
                    break;
                case kFlame:
                    typeStr = "Flame";
                    break;
                case kCone:
                    typeStr = "Cone";
                    break;
                case kArrow:
                    typeStr = "Arrow";
                    break;
                case kBeam:
                    typeStr = "Beam";
                    break;
                case kExplosion:
                    typeStr = "Explosion";
                    break;
                default:
                    break;
            }
            logger::debug("ProjectileUpdate called for projectile {} type {}", a_projectile->GetBaseObject()->GetName(),
                          typeStr);
                          */

            // auto start = std::chrono::high_resolution_clock::now();
            static auto set = Settings::GetSingleton();
            if (!set->ModActive) {
                return;
            }

            auto& runtimeData = a_projectile->GetProjectileRuntimeData();
            float now = runtimeData.livingTime;
            float interval = set->updateInterval;
            auto projFormID = a_projectile->GetFormID();
            auto it = lastProcessTime.find(projFormID);
            if (it != lastProcessTime.end()) {
                if (now - it->second < interval) {
                    return;
                }
            }

            if (!a_projectile->IsDisabled() && !a_projectile->IsDeleted()) {
                auto damageType = Utils::GetProjectileType(a_projectile);
                if (damageType == ElementType::Cold || damageType == ElementType::Water) {
                    RE::NiPoint3 startPos = a_projectile->GetPosition();

                    const auto waterHeight = Utils::get_water_height(a_projectile, startPos);
                    bool isLava = isWaterLava(a_projectile->GetParentCell(), waterHeight);

                    if (!isLava && damageType == ElementType::Water) {
                        return;  // Skip water projectiles in non-lava water
                    }

                    float projectileHeight = a_projectile->GetHeight();
                    if (projectileHeight <= 0.0f) {
                        projectileHeight = 1.0f;  // Default height if not set
                    }
                    if (type == kFlame || type == kBeam) {
                        RE::NiPoint3 endPos;
                        if (type == kFlame) {
                            RE::NiPoint3 linearVelocity;
                            a_projectile->GetLinearVelocity(linearVelocity);
                            linearVelocity.Unitize();

                            auto range = runtimeData.range;
                            endPos = startPos + linearVelocity * range;
                        } else {
                            const auto root = a_projectile->Get3D();
                            if (!root) {
                                return;
                            }
                            const auto beamEnd = root->GetObjectByName("BeamEnd");
                            if (!beamEnd) {
                                logger::warn("BeamEnd node not found for projectile {}", a_projectile->GetFormID());
                                return;
                            }
                            endPos = beamEnd->world.translate;
                        }

                        // DebugAPI_IMPL::DebugAPI::GetSingleton()->DrawLineForMS(startPos, endPos, 5000, {0, 1, 1, 1});


                        // logger::debug("Projectile {} water height: {}, startPos.z: {}, endPos.z: {}",
                        //               a_projectile->GetFormID(), waterHeight, startPos.z, endPos.z);
                        if (endPos.z != startPos.z) {
                            const auto t = (waterHeight - startPos.z) / (endPos.z - startPos.z);
                            endPos.x = (endPos.x - startPos.x) * t + startPos.x;
                            endPos.y = (endPos.y - startPos.y) * t + startPos.y;
                        }

                        if (!(waterHeight == -RE::NI_INFINITY)) {
                            const auto level = (waterHeight - endPos.z) / projectileHeight;
                            // logger::debug("Projectile {} water level: {}, water height: {}, projectile height: {},
                            // endPos.z: {}",
                            //     a_projectile->GetFormID(), level, waterHeight, projectileHeight, endPos.z);
                            if (level >= 0.1f) {
                                endPos.z = waterHeight;
                                auto damage = Utils::GetDamageFromProjectile(a_projectile);
                                // logger::debug("Projectile {} hit water at position {}, {}, {}, spawning ice chunk",
                                //     a_projectile->GetFormID(), endPos.x, endPos.y, endPos.z);
                                spawnIceChunk(damage, endPos, isLava);
                            }
                        }
                    } else {
                        if (!(waterHeight == -RE::NI_INFINITY)) {
                            float treshold = a_projectile->GetHeight() + 50.0f;
                            if (startPos.z > waterHeight - treshold && startPos.z < waterHeight + treshold) {
                                RE::NiPoint3 spawnPos{startPos.x, startPos.y, waterHeight};
                                // DebugAPI_IMPL::DebugAPI::GetSingleton()->DrawLineForMS(startPos, spawnPos, 5000,
                                //                                                        {0, 1, 1, 1});
                                auto damage = Utils::GetDamageFromProjectile(a_projectile);
                                spawnIceChunk(damage, spawnPos, isLava);
                                if (it != lastProcessTime.end()) {
                                    it->second = now;
                                } else {
                                    lastProcessTime[projFormID] = now;
                                }
                            }
                        }
                    }
                }
            }

            // auto end = std::chrono::high_resolution_clock::now();
            // std::chrono::duration<double, std::milli> elapsed = end - start;
            // logger::debug("Projectile Update took {} ms", elapsed.count());
        }

        void RemoveProjectile(RE::Projectile* a_projectile) { lastProcessTime.erase(a_projectile->GetFormID()); }

        void ProcessExplosion(RE::Explosion* a_explosion) {
            if (!a_explosion) {
                return;
            }
            // auto start = std::chrono::high_resolution_clock::now();
            static auto set = Settings::GetSingleton();
            if (!set->ModActive) {
                return;
            }

            auto damageType = Utils::GetExplosionType(a_explosion);
            if (damageType == ElementType::Cold || damageType == ElementType::Fire || damageType == ElementType::Water) {
                
                auto spawnPos = a_explosion->GetPosition();
                auto& explosionRuntimeData = a_explosion->GetExplosionRuntimeData();
                float radius = std::max(explosionRuntimeData.radius, 50.0f);
                float base_damage = explosionRuntimeData.damage;

                if (base_damage <= 0.0f) {
                    base_damage = radius / 10;
                }
                RE::NiColorA color{1, 0, 0, 1};

                for (auto& hazard : HazardIceChunks) {
                    if (auto hazardRef = hazard.second.handle.get().get()) {
                        auto HazardPos = hazardRef->GetPosition();
                        float distance = spawnPos.GetDistance(HazardPos);
                        if (distance < radius) {
                            float adjusted_damage = base_damage * (distance / radius);
                            DamageRef(hazardRef, adjusted_damage, damageType);
                        }
                    }
                }

                if (damageType == ElementType::Cold || damageType == ElementType::Water) {
                    color = {0, 1, 1, 1};
                    auto [waterAffectedRadius, waterHeight] = Utils::get_explosion_water_radius(a_explosion, radius);

                    if (waterHeight != -RE::NI_INFINITY && waterAffectedRadius > 0.0f) {
                        spawnPos.z = waterHeight;
                        bool isLava = isWaterLava(a_explosion->GetParentCell(), waterHeight);

                        if (waterAffectedRadius < 100) {
                            spawnIceChunk(base_damage, spawnPos, isLava);
                        } else {
                            spawnIceFloe(waterAffectedRadius, spawnPos, isLava);
                        }
                    }
                }
                // DebugAPI_IMPL::DebugAPI::GetSingleton()->DrawSphere(spawnPos, radius, 5000, color);
            }

            // auto end = std::chrono::high_resolution_clock::now();
            // std::chrono::duration<double, std::milli> elapsed = end - start;
            // logger::debug("Explosion took {} ms", elapsed.count());
        }

        void DamageRef(RE::TESObjectREFR* a_ref, float damage, ElementType type) {
            static auto* set = Settings::GetSingleton();

            float scaledDamage = damage / set->damageScale * 10.0f;
            if (type == ElementType::Cold) {
                if (auto hazard = a_ref->As<RE::Hazard>()) {
                    auto lifetime = hazard->GetHazardRuntimeData().lifetime;
                    auto age = hazard->GetHazardRuntimeData().age;
                    hazard->GetHazardRuntimeData().age = std::max(age - scaledDamage, 0.0f);
                    if (age - scaledDamage < 0.0f) {
                        hazard->GetHazardRuntimeData().lifetime =
                            std::min(lifetime + (scaledDamage - age), set->maxLifetime);
                    }
                    MeltObject(hazard, hazard->GetHazardRuntimeData().lifetime, hazard->GetHazardRuntimeData().age);
                }
            } else if (type == ElementType::Fire) {
                if (auto hazard = a_ref->As<RE::Hazard>()) {
                    auto lifetime = hazard->GetHazardRuntimeData().lifetime;
                    auto age = hazard->GetHazardRuntimeData().age;

                    float threshold = lifetime - set->meltingTime;

                    if (age < threshold) {
                        hazard->GetHazardRuntimeData().age = std::min(age + scaledDamage, threshold);
                    }
                }
            } else if (type == ElementType::Water) {
                // If we are here we know a_ref is from Frostwalker.esp, all we need is to check if its IceFloe or LavaFloe
                auto editorID = clib_util::editorID::get_editorID(a_ref->GetBaseObject());
                if (!editorID.empty()) {
                    std::string lowerStr = Utils::ToLower(editorID);
                    if (lowerStr.find("lava") != std::string::npos) {
                        if (auto hazard = a_ref->As<RE::Hazard>()) {
                            auto lifetime = hazard->GetHazardRuntimeData().lifetime;
                            auto age = hazard->GetHazardRuntimeData().age;

                            float threshold = lifetime - set->meltingTime;

                            if (age < threshold) {
                                hazard->GetHazardRuntimeData().age = std::min(age + scaledDamage, threshold);
                            }
                        }
                    }
                }
            }
        }

        void ProjectileImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref) {
            static auto set = Settings::GetSingleton();
            if (!set->ModActive) {
                return;
            }

            if (a_ref && a_proj) {
                // auto start = std::chrono::high_resolution_clock::now();

                std::string filename = "";
                if (auto base_obj = a_ref->GetBaseObject()) {
                    if (auto ModFile = base_obj->GetFile()) {
                        filename = ModFile->GetFilename();
                    }
                }
                if (filename == "Frostwalker.esp") {
                    auto damageType = Utils::GetProjectileType(a_proj);
                    auto damage = Utils::GetDamageFromProjectile(a_proj);
                    DamageRef(a_ref, damage, damageType);
                }

                // auto end = std::chrono::high_resolution_clock::now();
                // std::chrono::duration<double, std::milli> elapsed = end - start;
                // logger::debug("Projectile Impact took {} ms", elapsed.count());
            }
        }

    private:

        bool isWaterLava(RE::TESObjectCELL* cell, float waterHeight) {
            bool isLava = false;

            if (cell) {
                if (waterHeight < cell->GetRuntimeData().waterHeight + 10 &&
                    waterHeight > cell->GetRuntimeData().waterHeight - 10) {
                    if (auto* extra = cell->extraList.GetByType<RE::ExtraCellWaterType>()) {
                        if (auto* waterForm = extra->water) {
                            auto editorID = clib_util::editorID::get_editorID(waterForm);
                            if (!editorID.empty()) {
                                std::string lowerStr = Utils::ToLower(editorID);
                                if (lowerStr.find("lava") != std::string::npos) {
                                    isLava = true;
                                }
                            }
                        }
                    }
                }
            }
            return isLava;
        }

        void spawnIceChunk(float damage, RE::NiPoint3 SpawnPos, bool isLava, bool Actor = false) {
            if (damage <= 0.0f) {
                return;
            }
            SKSE::GetTaskInterface()->AddTask([this, damage, SpawnPos, isLava, Actor] {
                static auto set = Settings::GetSingleton();
                static auto player = RE::PlayerCharacter::GetSingleton();

                if (!Actor) {
                    for (auto& hazard : HazardIceChunks) {
                        if (auto hazardRef = hazard.second.handle.get().get()) {
                            auto HazardPos = hazardRef->GetPosition();
                            float distance = SpawnPos.GetDistance(HazardPos);
                            if (distance < set->IceDistance) {
                                return;
                            }
                        }
                    }
                }

                std::vector<RE::BGSHazard*> Floes;

                if (isLava)
                    Floes = set->LavaChunks;
                else
                    Floes = set->IceChunks;

                if (Floes.empty()) {
                    logger::warn("No Floes defined in settings");
                    return;
                }

                int iceChunk = 0;
                if (!Actor) {
                    int maxIndex =static_cast<int>(Floes.size()) - 1;
                    iceChunk = RandomInt(0, maxIndex);
                }

                RE::TESBoundObject* objToSpawn = Floes[static_cast<std::size_t>(iceChunk)];
                if (!objToSpawn) {
                    logger::error("Selected IceChunk object is null");
                    return;
                }

                auto spawnedRef = player->PlaceObjectAtMe(objToSpawn, false);
                if (!spawnedRef) {
                    logger::error("Failed to spawn ice chunk");
                    return;
                }
                // Random rotation around Z-axis (yaw) to add visual variety
                float yaw = RandomFloat(-M_PI_F, M_PI_F);
                float scale = Actor ? set->maxScale :std::clamp(damage / set->damageScale, set->minScale, set->maxScale);

                RE::NiPoint3 finalSpawnPos = SpawnPos;
                finalSpawnPos.z += RandomFloat(-1.0f, 1.0f);  // Slightly randomize the Z position for fixing z fighting

                if (isLava) {
                    finalSpawnPos.z += 20.0f;  // Raise the floe above lava
                }

                spawnedRef->SetPosition(finalSpawnPos);
                spawnedRef->SetAngle(RE::NiPoint3{0.0f, 0.0f, yaw});
                spawnedRef->SetScale(scale);

                if (auto spawnedHazard = spawnedRef->As<RE::Hazard>()) {
                    spawnedHazard->GetHazardRuntimeData().lifetime =
                        std::clamp(damage / 2.0f, set->minLifetime, set->maxLifetime);
                }

                IceSize chunkSize = IceSize::Chunk;

                HazardIceChunks[spawnedRef->GetFormID()] = {spawnedRef->GetHandle(), chunkSize};
            });
        }

        void spawnIceFloe(float radius, RE::NiPoint3 SpawnPos, bool isLava) {
            SKSE::GetTaskInterface()->AddTask([this, radius, SpawnPos, isLava] {
                static auto set = Settings::GetSingleton();
                static auto player = RE::PlayerCharacter::GetSingleton();

                std::vector<RE::BGSHazard*> Floes;

                IceSize newSize;

                if (radius < 350) {
                    newSize = IceSize::Small;
                    if (isLava)
                        Floes = set->LavaFloesS;
                    else
                        Floes = set->IceFloesS;
                } else if (radius < 700) {
                    newSize = IceSize::Medium;
                    if (isLava)
                        Floes = set->LavaFloesM;
                    else
                        Floes = set->IceFloesM;
                } else {
                    newSize = IceSize::Large;
                    if (isLava)
                        Floes = set->LavaFloesL;
                    else
                        Floes = set->IceFloesL;
                }

                if (Floes.empty()) {
                    logger::warn("No Floes defined in settings");
                    return;
                }

                int maxIndex = static_cast<int>(Floes.size()) - 1;
                if (maxIndex < 0) {
                    logger::error("IceFloe size underflow");
                    return;
                }
                int iceChunk = RandomInt(0, maxIndex);

                RE::TESBoundObject* objToSpawn = Floes[static_cast<std::size_t>(iceChunk)];
                if (!objToSpawn) {
                    logger::error("Selected IceFloe object is null");
                    return;
                }

                for (auto& hazard : HazardIceChunks) {
                    if (auto hazardRef = hazard.second.handle.get().get()) {
                        auto HazardPos = hazardRef->GetPosition();
                        float distance = SpawnPos.GetDistance(HazardPos);
                        if (distance < set->IceDistance) {
                            if (!CanMerge(hazard.second.size, newSize)) {
                                return;
                            }
                        }
                    }
                }

                auto spawnedRef = player->PlaceObjectAtMe(objToSpawn, false);
                if (!spawnedRef) {
                    logger::error("Failed to spawn ice floe");
                    return;
                }
                // Random rotation around Z-axis (yaw) to add visual variety
                float yaw = RandomFloat(-M_PI_F, M_PI_F);

                RE::NiPoint3 finalSpawnPos = SpawnPos;
                finalSpawnPos.z += RandomFloat(-1.0f, 1.0f);  // Slightly randomize the Z position for variety

                if (isLava) {
                    finalSpawnPos.z += 20.0f;  // Raise the floe above lava
                }

                spawnedRef->SetPosition(finalSpawnPos);
                spawnedRef->SetAngle(RE::NiPoint3{0.0f, 0.0f, yaw});

                if (auto spawnedHazard = spawnedRef->As<RE::Hazard>()) {
                    spawnedHazard->GetHazardRuntimeData().lifetime = std::min(radius / 10.0f, set->maxLifetime);
                }

                HazardIceChunks[spawnedRef->GetFormID()] = {spawnedRef->GetHandle(), newSize};
            });
        }

        bool CanMerge(IceSize existing, IceSize incoming) {
            switch (incoming) {
                case IceSize::Chunk:
                    return false;

                case IceSize::Small:
                    return existing == IceSize::Chunk;

                case IceSize::Medium:
                    return existing == IceSize::Chunk || existing == IceSize::Small;

                case IceSize::Large:
                    return existing == IceSize::Chunk || existing == IceSize::Small || existing == IceSize::Medium;
            }

            return false;
        }

        std::unordered_map<RE::FormID, IceHazardData> HazardIceChunks;
        std::unordered_map<RE::FormID, float> lastProcessTime;
    };
}