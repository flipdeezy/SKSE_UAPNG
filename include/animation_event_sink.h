#pragma once

struct AnimationEventSink;

void ProcessPotionAnimationEvent(const AnimationEventSink* eventSink);
std::vector<AnimationEventSink*> GetAllEventSinks();
AnimationEventSink* GetOrCreateEventSink(RE::Actor* a_actor, RE::ActorEquipManager* a_self,
                                         RE::TESBoundObject* a_object, std::uint64_t a_unk);
void CleanupAnimationEvent(AnimationEventSink* eventSink, bool unHideWeapon);
void RemoveEventSink(const AnimationEventSink* sink);

struct ActorCustom
{
    RE::FormID formid;
    RE::FormID refid;

    ActorCustom(RE::FormID formid, RE::FormID refid);
    ActorCustom(RE::Actor* a_actor);

    bool operator==(const ActorCustom& other) const;

    bool operator==(const RE::Actor* actor) const;

    bool operator<(const ActorCustom& other) const;

    explicit operator bool() const {return get()!=nullptr;}

    [[nodiscard]] RE::Actor* get() const;
};

inline std::map<ActorCustom, std::unique_ptr<AnimationEventSink>> actorEventSink;

struct AnimationEventSink : public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
    AnimationEventSink(RE::ActorEquipManager* equipManager, RE::Actor* actor, RE::TESBoundObject* object,
                       std::uint64_t unk);
    RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event,
                                          RE::BSTEventSource<RE::BSAnimationGraphEvent>*) override;

    RE::ActorEquipManager* a_equipManager{nullptr};
    ActorCustom a_actor;
    RE::TESBoundObject* a_object{nullptr};
    std::uint64_t a_unk{0};
    std::atomic<bool> marked_for_delete = false;
};
