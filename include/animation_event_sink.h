#pragma once

struct AnimationEventSink;

void ProcessPotionAnimationEvent(AnimationEventSink* eventSink);
std::vector<AnimationEventSink*> GetAllEventSinks();
AnimationEventSink* GetOrCreateEventSink(RE::Actor* a_actor, RE::ActorEquipManager* a_self,
                                         RE::TESBoundObject* a_object, std::uint64_t a_unk);
void CleanupAnimationEvent(AnimationEventSink* eventSink, bool unHideWeapon);
void RemoveEventSink(AnimationEventSink* sink);

extern std::unordered_map<RE::Actor*, std::unique_ptr<AnimationEventSink>> actorEventSink;

struct AnimationEventSink : public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
    AnimationEventSink(RE::ActorEquipManager* equipManager, RE::Actor* actor, RE::TESBoundObject* object,
                       std::uint64_t unk);
    RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event,
                                          RE::BSTEventSource<RE::BSAnimationGraphEvent>*);

    RE::ActorEquipManager* a_equipManager{nullptr};
    RE::Actor* a_actor{nullptr};
    RE::TESBoundObject* a_object{nullptr};
    std::uint64_t a_unk{0};
};