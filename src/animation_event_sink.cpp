#include "actor_data.h"
#include "animation_event_sink.h"
#include "function_library.h"
#include "logger.h"

std::unordered_map<RE::Actor*, std::unique_ptr<AnimationEventSink>> actorEventSink;

AnimationEventSink::AnimationEventSink(RE::ActorEquipManager* equipManager, RE::Actor* actor,
                                       RE::TESBoundObject* object, std::uint64_t unk)
    : a_equipManager(equipManager), a_actor(actor), a_object(object), a_unk(unk) {}

RE::BSEventNotifyControl AnimationEventSink::ProcessEvent(
    const RE::BSAnimationGraphEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::BSAnimationGraphEvent>* source) {
    if (a_event) {
        const RE::BSFixedString& eventTag = a_event->tag;
        if (eventTag == "IUHeal") {            
            ActorData& data = actorDataMap[a_actor];
            data.bDrinking = false;
            ProcessPotionAnimationEvent(this);
            
        } else if (eventTag == "DrinkPotionStop" || eventTag == "EnableBumper" || eventTag == "MTState") {
            CleanupAnimationEvent(this, true);
            RemoveEventSink(this);
        }
    }
    return RE::BSEventNotifyControl::kContinue;
}

void ProcessPotionAnimationEvent(AnimationEventSink* eventSink) {
    logger::info("Processing potion animation event for Actor: '{}'", eventSink->a_actor->GetName());
    eventSink->a_equipManager->EquipObject(eventSink->a_actor, eventSink->a_object);
}

AnimationEventSink* GetOrCreateEventSink(RE::Actor* a_actor, RE::ActorEquipManager* a_self,
                                         RE::TESBoundObject* a_object, std::uint64_t a_unk) {
    AnimationEventSink* eventSink = nullptr;
    auto it = actorEventSink.find(a_actor);
    if ((a_actor) && (it != actorEventSink.end())) {
        eventSink = it->second.get();
    } else {
        eventSink = new AnimationEventSink(a_self, a_actor, a_object, a_unk);
        actorEventSink[a_actor] = std::unique_ptr<AnimationEventSink>(eventSink);
    }
    return eventSink;
}

std::vector<AnimationEventSink*> GetAllEventSinks() {
    std::vector<AnimationEventSink*> eventSinks;
    for (auto& pair : actorEventSink) {
        eventSinks.push_back(pair.second.get());
    }
    return eventSinks;
}

void CleanupAnimationEvent(AnimationEventSink* eventSink, bool unHideWeapon) {
    if (!eventSink || !eventSink->a_actor) {
        logger::error("Error ++++++++++++++++++++++ CleanupAnimationEvent Failed: eventSink or eventSink->a_actor is null.");
        return;
    }

    ActorData& data = actorDataMap[eventSink->a_actor];
    data.bAnimationInProgress = false;
    data.bDrinking = false;

    if (unHideWeapon) {
        HideWeapon(eventSink->a_actor, false);
    }

    SendAnimationEvent(eventSink->a_actor, "HeadtrackingOn");
    SetAnimationVariableBool(eventSink->a_actor, "tdmHeadtrackingBehavior", true);
    SetAnimationVariableBool(eventSink->a_actor, "bPlayEquipSound", false);

    eventSink->a_actor->RemoveAnimationGraphEventSink(eventSink);
}

void RemoveEventSink(AnimationEventSink* sink) {
    if (!sink) return;
    auto it = actorEventSink.find(sink->a_actor);
    if (it != actorEventSink.end()) {
        actorEventSink.erase(it);
    }
}
