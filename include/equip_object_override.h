#pragma once

class EquipObjectOverRide {
public:
    static void thunk(RE::ActorEquipManager* a_self, RE::Actor* a_actor, RE::TESBoundObject* a_object, std::uint64_t a_unk);
    static inline REL::Relocation<decltype(thunk)> func;
    static void Install();
};
