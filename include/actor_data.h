#pragma once

struct ActorData {
    ActorData() : lastActor(nullptr), lastObject(nullptr), bAnimationInProgress(false), bDrinking(false) {}

    RE::Actor* lastActor;
    RE::TESBoundObject* lastObject;
    bool bAnimationInProgress;
    bool bDrinking;
};

extern std::unordered_map<RE::Actor*, ActorData> actorDataMap;

enum class ActorState { PLAYER_FIRST_PERSON, PLAYER_THIRD_PERSON, OTHER };