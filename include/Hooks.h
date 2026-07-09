#pragma once

namespace Hooks {

    struct UpdatePlayerHook {
        static void Update(RE::Actor* a_this, float a_delta);
        static inline REL::Relocation<decltype(Update)> Update_;
    };

    struct UpdateActorHook {
        static void Update(RE::Actor* a_this, float a_delta);
        static inline REL::Relocation<decltype(Update)> Update_;
    };

    struct MissileProjectile {
        static void Update(RE::Projectile* a_proj, float a_delta);
        static inline REL::Relocation<decltype(Update)> Update_;

        static void AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                              const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                              std::uint32_t a_arg7);
        static inline REL::Relocation<decltype(AddImpact)> AddImpact_;

        static void SetDelete(RE::Projectile* a_proj, bool a_set);
        static inline REL::Relocation<decltype(SetDelete)> SetDelete_;
    };
    struct BeamProjectile {
        static void Update(RE::Projectile* a_proj, float a_delta);
        static inline REL::Relocation<decltype(Update)> Update_;

        static void AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                              const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                              std::uint32_t a_arg7);
        static inline REL::Relocation<decltype(AddImpact)> AddImpact_;

        static void SetDelete(RE::Projectile* a_proj, bool a_set);
        static inline REL::Relocation<decltype(SetDelete)> SetDelete_;
    };
    struct FlameProjectile {
        static void Update(RE::Projectile* a_proj, float a_delta);
        static inline REL::Relocation<decltype(Update)> Update_;

        static void AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                              const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                              std::uint32_t a_arg7);
        static inline REL::Relocation<decltype(AddImpact)> AddImpact_;

        static void SetDelete(RE::Projectile* a_proj, bool a_set);
        static inline REL::Relocation<decltype(SetDelete)> SetDelete_;
    };
    struct GrenadeProjectile {
        static void Update(RE::Projectile* a_proj, float a_delta);
        static inline REL::Relocation<decltype(Update)> Update_;

        static void AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                              const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                              std::uint32_t a_arg7);
        static inline REL::Relocation<decltype(AddImpact)> AddImpact_;

        static void SetDelete(RE::Projectile* a_proj, bool a_set);
        static inline REL::Relocation<decltype(SetDelete)> SetDelete_;
    };
    struct ConeProjectile {
        static void Update(RE::Projectile* a_proj, float a_delta);
        static inline REL::Relocation<decltype(Update)> Update_;

        static void AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                              const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                              std::uint32_t a_arg7);
        static inline REL::Relocation<decltype(AddImpact)> AddImpact_;

        static void SetDelete(RE::Projectile* a_proj, bool a_set);
        static inline REL::Relocation<decltype(SetDelete)> SetDelete_;
    };
    struct ArrowProjectile {
        static void Update(RE::Projectile* a_proj, float a_delta);
        static inline REL::Relocation<decltype(Update)> Update_;

        static void AddImpact(RE::Projectile* a_proj, RE::TESObjectREFR* a_ref, const RE::NiPoint3& a_targetLoc,
                              const RE::NiPoint3& a_velocity, RE::hkpCollidable* a_collidable, std::int32_t a_arg6,
                              std::uint32_t a_arg7);
        static inline REL::Relocation<decltype(AddImpact)> AddImpact_;

        static void SetDelete(RE::Projectile* a_proj, bool a_set);
        static inline REL::Relocation<decltype(SetDelete)> SetDelete_;
    };

    struct ExplosionHook {
        static void Initialize(RE::Explosion* a_this);
        static inline REL::Relocation<decltype(Initialize)> Initialize_;
    };

    inline void InstallHooks() {
        UpdatePlayerHook::Update_ = REL::Relocation<std::uintptr_t>(RE::VTABLE_PlayerCharacter[0])
                                        .write_vfunc(REL::Relocate(0xAD, 0xAD, 0xAF), UpdatePlayerHook::Update);

        UpdateActorHook::Update_ = REL::Relocation<std::uintptr_t>(RE::VTABLE_Character[0])
                                       .write_vfunc(REL::Relocate(0xAD, 0xAD, 0xAF), UpdateActorHook::Update);


        MissileProjectile::Update_ = REL::Relocation<std::uintptr_t>(RE::MissileProjectile::VTABLE[0])
                                         .write_vfunc(0xAB, MissileProjectile::Update);
        BeamProjectile::Update_ =
            REL::Relocation<std::uintptr_t>(RE::BeamProjectile::VTABLE[0]).write_vfunc(0xAB, BeamProjectile::Update);
        FlameProjectile::Update_ =
            REL::Relocation<std::uintptr_t>(RE::FlameProjectile::VTABLE[0]).write_vfunc(0xAB, FlameProjectile::Update);
        // GrenadeProjectile::Update_ = REL::Relocation<std::uintptr_t>(RE::GrenadeProjectile::VTABLE[0])
        //                                  .write_vfunc(0xAB, GrenadeProjectile::Update);
        // ArrowProjectile::Update_ =
        //    REL::Relocation<std::uintptr_t>(RE::ArrowProjectile::VTABLE[0]).write_vfunc(0xAB, ArrowProjectile::Update);
        ConeProjectile::Update_ =
            REL::Relocation<std::uintptr_t>(RE::ConeProjectile::VTABLE[0]).write_vfunc(0xAB, ConeProjectile::Update);

        MissileProjectile::AddImpact_ = REL::Relocation<std::uintptr_t>(RE::MissileProjectile::VTABLE[0])
                                            .write_vfunc(0xBD, MissileProjectile::AddImpact);
        BeamProjectile::AddImpact_ =
            REL::Relocation<std::uintptr_t>(RE::BeamProjectile::VTABLE[0]).write_vfunc(0xBD, BeamProjectile::AddImpact);
        FlameProjectile::AddImpact_ = REL::Relocation<std::uintptr_t>(RE::FlameProjectile::VTABLE[0])
                                          .write_vfunc(0xBD, FlameProjectile::AddImpact);
        // GrenadeProjectile::AddImpact_ = REL::Relocation<std::uintptr_t>(RE::GrenadeProjectile::VTABLE[0])
        //                                     .write_vfunc(0xBD, GrenadeProjectile::AddImpact);
        ArrowProjectile::AddImpact_ = REL::Relocation<std::uintptr_t>(RE::ArrowProjectile::VTABLE[0])
                                          .write_vfunc(0xBD, ArrowProjectile::AddImpact);
        ConeProjectile::AddImpact_ =
            REL::Relocation<std::uintptr_t>(RE::ConeProjectile::VTABLE[0]).write_vfunc(0xBD, ConeProjectile::AddImpact);

        MissileProjectile::SetDelete_ = REL::Relocation<std::uintptr_t>(RE::MissileProjectile::VTABLE[0])
                                            .write_vfunc(0x23, MissileProjectile::SetDelete);
        BeamProjectile::SetDelete_ =
            REL::Relocation<std::uintptr_t>(RE::BeamProjectile::VTABLE[0]).write_vfunc(0x23, BeamProjectile::SetDelete);
        FlameProjectile::SetDelete_ = REL::Relocation<std::uintptr_t>(RE::FlameProjectile::VTABLE[0])
                                          .write_vfunc(0x23, FlameProjectile::SetDelete);
        // GrenadeProjectile::SetDelete_ = REL::Relocation<std::uintptr_t>(RE::GrenadeProjectile::VTABLE[0])
        //                                     .write_vfunc(0x23, GrenadeProjectile::SetDelete);
        ArrowProjectile::SetDelete_ = REL::Relocation<std::uintptr_t>(RE::ArrowProjectile::VTABLE[0])
                                          .write_vfunc(0x23, ArrowProjectile::SetDelete);
        ConeProjectile::SetDelete_ =
            REL::Relocation<std::uintptr_t>(RE::ConeProjectile::VTABLE[0]).write_vfunc(0x23, ConeProjectile::SetDelete);

        ExplosionHook::Initialize_ =
            REL::Relocation<std::uintptr_t>(RE::Explosion::VTABLE[0]).write_vfunc(0xA2, ExplosionHook::Initialize);
    }
}  // namespace Hooks