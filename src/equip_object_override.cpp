#include "equip_object_override.h"
#include "actor_data.h"
#include "function_library.h"
#include "logger.h"
#include "pch.h"
#include "config.h"

RE::TESBoundObject* lastEquippedObject;

void EquipObjectOverRide::thunk(RE::ActorEquipManager* a_self, RE::Actor* a_actor, RE::TESBoundObject* a_object, std::uint64_t a_unk) {

    if (pause.load()) {
        logger::warn("Pause is active, skipping EquipObjectOverRide::thunk");
        func(a_self, a_actor, a_object, a_unk);
        return;
    }

    if (!a_object || !a_actor || !a_self) {
        func(a_self, a_actor, a_object, a_unk);
        return;
    }

    if (IsObjectBlacklisted(a_object)) {
        func(a_self, a_actor, a_object, a_unk);
        return;
    }

    if (CheckActorAndObjectKeywords(a_object, a_actor) || IsObjectAPotion(a_object) || IsObjectWhitelisted(a_object)) {
        const auto actor_custom = ActorCustom(a_actor);
        ActorData& data = actorDataMap[actor_custom];
        if (IsActorReadyForAnimation(a_actor )) {
            data.bAnimationInProgress.store(true);
            data.bDrinking.store(true);
            AnimObjectReplaceModelByFormID(0x0D36C8, AlchObjectGetModelPath(a_object).c_str());
            EquipPotion(a_actor, a_self, a_object, a_unk);
            if (!config::bEnableSpamPotions) {
                CloseMenu();
            }
            lastEquippedObject = a_object;
        } else {            
            if (!config::bEnableSpamPotions){
                if (!data.bDrinking) {                
                    data.bDrinking = true;
                    NotifyActorDrinkingLastPotion(a_actor, a_object);
                    func(a_self, a_actor, a_object, a_unk);
                } else {
                    if (a_actor->GetActorBase()->IsPlayer()) {
                        SendDebugNotification("You can't use a potion right now");
                    }
                }
            } else {
                NotifyActorDrinkingLastPotion(a_actor, a_object);
                func(a_self, a_actor, a_object, a_unk);
            }                             
        }
    } else {
        func(a_self, a_actor, a_object, a_unk);
    }
}

void EquipObjectOverRide::Install() {
    REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37938, 38894), REL::VariantOffset(0xE5, 0x170, 0xE5) };
    stl::write_thunk_call<EquipObjectOverRide>(target.address());
    logger::info("Ultimate Potion Animation NG - Hook Installed");
}
