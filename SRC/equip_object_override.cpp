#include "actor_data.h"
#include "equip_object_override.h"
#include "function_library.h"
#include "logger.h"
#include "PCH.h"

RE::TESBoundObject* lastEquippedObject;

void EquipObjectOverRide::thunk(RE::ActorEquipManager* a_self, RE::Actor* a_actor, RE::TESBoundObject* a_object,
                                std::uint64_t a_unk) {

    if (CheckActorAndObjectKeywords(a_object, a_actor) || IsObjectAPotion(a_object)) {
        SKSE::log::debug("Ultimate Potion Animation NG - EquipObjectOverRide - Actor: {}, Object: {}",
                         a_actor->GetName(), a_object->GetName());

        ActorData& data = actorDataMap[a_actor];

        if (IsActorReadyForAnimation(a_actor)) {
            data.bAnimationInProgress = true;
            AnimObjectReplaceModelByFormID(0x0D36C8, AlchObjectGetModelPath(a_object).c_str());
            EquipPotion(a_actor, a_self, a_object, a_unk);
            CloseMenu();

            lastEquippedObject = a_object;

        } else {
            // Check if conditions not met because the actor already equipped the potion
            if (data.bAnimationInProgress) {
                // Check if the actor still needs to drink the equipped potion
                if (!data.bDrinking) {
                    // drink the potion
                    func(a_self, a_actor, lastEquippedObject, a_unk);
                    data.bDrinking = true;
                } else {
                    // Actor drank the equipped potion and needs to wait for the animation to finish
                    NotifyActorDrinkingLastPotion(a_actor);
                }
            } else {
                // Let the player know conditions not met but the actor has not equipped a potion yet
                if (a_actor->GetActorBase()->IsPlayer()) {
                    SendDebugNotification("You can't use a potion right now");
                }
            }
        }
    } else {
        // the actor and object don't meet the criteria, so proceed with the original function
        func(a_self, a_actor, a_object, a_unk);
    }
}

void EquipObjectOverRide::Install() {
    REL::Relocation<std::uintptr_t> target{RELOCATION_ID(37938, 38894), REL::VariantOffset(0xE5, 0x170, 0xE5)};
    stl::write_thunk_call<EquipObjectOverRide>(target.address());
    SKSE::log::info("Ultimate Potion Animation NG - Hook Installed");
}
