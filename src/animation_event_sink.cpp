#include "actor_data.h"
#include "animation_event_sink.h"
#include "function_library.h"
#include "logger.h"

AnimationEventSink::AnimationEventSink(RE::ActorEquipManager* equipManager, RE::Actor* actor,
                                       RE::TESBoundObject* object, std::uint64_t unk)
    : a_equipManager(equipManager), a_actor(actor), a_object(object), a_unk(unk) {}

RE::BSEventNotifyControl AnimationEventSink::ProcessEvent(
    const RE::BSAnimationGraphEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::BSAnimationGraphEvent>* source) {
    if (marked_for_delete.load()) return RE::BSEventNotifyControl::kContinue;
    if (a_event && a_actor) {
        if (const RE::BSFixedString& eventTag = a_event->tag; eventTag == "IUHeal") {            
            ActorData& data = actorDataMap[a_actor];
            data.bDrinking.store(false);
            ProcessPotionAnimationEvent(this);
            
        } else if (eventTag == "DrinkPotionStop" || eventTag == "EnableBumper" || eventTag == "MTState") {
            logger::trace("AnimationEventSink::ProcessEvent:");
            logger::trace("Removing event sink for Actor: '{}'", a_actor.get()->GetName());
            CleanupAnimationEvent(this, true);
            RemoveEventSink(this);
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

void ProcessPotionAnimationEvent(const AnimationEventSink* eventSink) {
    if (!eventSink->a_actor) return;
    logger::info("Processing potion animation event for Actor: '{}'", eventSink->a_actor.get()->GetName());
    eventSink->a_equipManager->EquipObject(eventSink->a_actor.get(), eventSink->a_object);
}

AnimationEventSink* GetOrCreateEventSink(RE::Actor* a_actor, RE::ActorEquipManager* a_self,
                                         RE::TESBoundObject* a_object, std::uint64_t a_unk) {
    AnimationEventSink* eventSink = nullptr;
    if (!a_actor) return eventSink;
    const auto actor_custom = ActorCustom(a_actor);
    if (const auto it = actorEventSink.find(actor_custom); it != actorEventSink.end()) {
        eventSink = it->second.get();
    } else {
        eventSink = new AnimationEventSink(a_self, a_actor, a_object, a_unk);
        actorEventSink.emplace(a_actor, std::unique_ptr<AnimationEventSink>(eventSink));
    }
    return eventSink;
}

void CleanupAnimationEvent(AnimationEventSink* eventSink, const bool unHideWeapon) {
    if (!eventSink || !eventSink->a_actor) {
        logger::warn("Error ++++++++++++++++++++++ CleanupAnimationEvent Failed: eventSink or eventSink->a_actor is null.");
        return;
    }


    logger::trace("Cleaning up AnimationEventSink for Actor: '{}'", eventSink->a_actor.get()->GetName());

    eventSink->marked_for_delete.store(true);

    ActorData& data = actorDataMap[eventSink->a_actor];
    data.bAnimationInProgress.store(false);
    data.bDrinking.store(false);

    if (unHideWeapon) {
        HideWeapon(eventSink->a_actor.get(), false);
    }

    SendAnimationEvent(eventSink->a_actor.get(), "HeadtrackingOn");
    SetAnimationVariableBool(eventSink->a_actor.get(), "tdmHeadtrackingBehavior", true);
    SetAnimationVariableBool(eventSink->a_actor.get(), "bPlayEquipSound", false);

    eventSink->a_actor.get()->RemoveAnimationGraphEventSink(eventSink);

    logger::trace("EventSink removed.");

}

void RemoveEventSink(const AnimationEventSink* sink) {
    if (!sink) return;
    if (const auto it = actorEventSink.find(sink->a_actor); it != actorEventSink.end()) {
        actorEventSink.erase(it);
    }
    logger::info("AnimationEventSink deleted.");
}

ActorCustom::ActorCustom(RE::FormID formid, RE::FormID refid): formid(formid), refid(refid) {}

ActorCustom::ActorCustom(RE::Actor* a_actor)
{
    refid = a_actor ? a_actor->GetFormID() : 0;
    formid = a_actor ? a_actor->GetBaseObject()->GetFormID() : 0;
}

bool ActorCustom::operator==(const ActorCustom& other) const {
    return formid == other.formid && refid == other.refid;
}

bool ActorCustom::operator==(const RE::Actor* actor) const {
    if (!actor) return false;
    return refid == actor->GetFormID() && refid == actor->GetBaseObject()->GetFormID();
}

bool ActorCustom::operator<(const ActorCustom& other) const {
    return refid < other.refid;
}

RE::Actor* ActorCustom::get() const {
    auto* actor_temp = RE::TESForm::LookupByID<RE::Actor>(refid);
    return actor_temp && actor_temp->GetBaseObject()->GetFormID() == formid ? actor_temp : nullptr;
}
