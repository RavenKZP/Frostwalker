#include "Hooks.h"

#include "Manager.h"
#include "Settings.h"

namespace Hooks {

    void UpdatePlayerHook::Update(RE::Actor* a_this, float a_delta) {
        Update_(a_this, a_delta);

        static auto* set = Settings::GetSingleton();
        if (!set->ModActive) {
            return;
        }

        Frostwalker::Manager::GetSingleton()->FrameUpdate(a_delta);
        Frostwalker::Manager::GetSingleton()->FrostWalk(a_this);

    }

    void UpdateActorHook::Update(RE::Actor* a_this, float a_delta) {
        Update_(a_this, a_delta);

        static auto* set = Settings::GetSingleton();
        if (!set->ModActive) {
            return;
        }
        Frostwalker::Manager::GetSingleton()->FrostWalk(a_this);
    }

    // Projectile Updates
    void MissileProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Frostwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kMissile);
        Update_(a_proj, a_delta);
    }

    void BeamProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Frostwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kBeam);
        Update_(a_proj, a_delta);
    }

    void FlameProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Frostwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kFlame);
        Update_(a_proj, a_delta);
    }

    void GrenadeProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Frostwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kExplosion);
        Update_(a_proj, a_delta);
    }

    void ConeProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Frostwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kCone);
        Update_(a_proj, a_delta);
    }

    void ArrowProjectile::Update(RE::Projectile* a_proj, float a_delta) {
        Frostwalker::Manager::GetSingleton()->ProjectileUpdate(a_proj, TYPE::kArrow);
        Update_(a_proj, a_delta);
    }

    // Projectile Impacts
    void MissileProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                      const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable,
                                      std::int32_t a_arg6, std::uint32_t a_arg7) {
        Frostwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void BeamProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                   const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                                   std::uint32_t a_arg7) {
        Frostwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void FlameProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                    const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable,
                                    std::int32_t a_arg6, std::uint32_t a_arg7) {
        a_proj->GetProjectileRuntimeData().linearVelocity = a_targetLoc - a_proj->GetPosition();
        Frostwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void GrenadeProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                      const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable,
                                      std::int32_t a_arg6, std::uint32_t a_arg7) {
        Frostwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void ConeProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                   const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                                   std::uint32_t a_arg7) {
        Frostwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    void ArrowProjectile::AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                                    const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable,
                                    std::int32_t a_arg6, std::uint32_t a_arg7) {
        Frostwalker::Manager::GetSingleton()->ProjectileImpact(a_proj, a_ref);
        AddImpact_(a_proj, a_ref, a_targetLoc, a_velocity, a_collidable, a_arg6, a_arg7);
    }

    // Projectile Deletion
    void MissileProjectile::SetDelete(RE::Projectile* a_proj, bool a_set) {
        if (a_proj && a_set) {
            Frostwalker::Manager::GetSingleton()->RemoveProjectile(a_proj);
        }
        SetDelete_(a_proj, a_set);
    }
    void BeamProjectile::SetDelete(RE::Projectile* a_proj, bool a_set) {
        if (a_proj && a_set) {
            Frostwalker::Manager::GetSingleton()->RemoveProjectile(a_proj);
        }
        SetDelete_(a_proj, a_set);
    }
    void GrenadeProjectile::SetDelete(RE::Projectile* a_proj, bool a_set) {
        if (a_proj && a_set) {
            Frostwalker::Manager::GetSingleton()->RemoveProjectile(a_proj);
        }
    }
    void FlameProjectile::SetDelete(RE::Projectile* a_proj, bool a_set) {
        if (a_proj && a_set) {
            Frostwalker::Manager::GetSingleton()->RemoveProjectile(a_proj);
        }
        SetDelete_(a_proj, a_set);
    }
    void ConeProjectile::SetDelete(RE::Projectile* a_proj, bool a_set) {
        if (a_proj && a_set) {
            Frostwalker::Manager::GetSingleton()->RemoveProjectile(a_proj);
        }
        SetDelete_(a_proj, a_set);
    }
    void ArrowProjectile::SetDelete(RE::Projectile* a_proj, bool a_set) {
        if (a_proj && a_set) {
            Frostwalker::Manager::GetSingleton()->RemoveProjectile(a_proj);
        }
        SetDelete_(a_proj, a_set);
    }

    // Explosions
    void ExplosionHook::Initialize(RE::Explosion* a_this) {
        Frostwalker::Manager::GetSingleton()->ProcessExplosion(a_this);
        Initialize_(a_this);
    }
}  // namespace Hooks