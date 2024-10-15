#pragma once
#include "animation_event_sink.h"

struct ActorData {
    ActorData() : lastActor(nullptr), lastObject(nullptr), bAnimationInProgress(false), bDrinking(false) {}

    RE::Actor* lastActor;
    RE::TESBoundObject* lastObject;
    std::atomic<bool> bAnimationInProgress;
    std::atomic<bool> bDrinking;
};

inline std::map<ActorCustom, ActorData> actorDataMap;

enum class ActorState { PLAYER_FIRST_PERSON, PLAYER_THIRD_PERSON, OTHER };
