#pragma once

void SendDebugNotification(const char* message, const char* soundToPlay = nullptr, bool cancelIfAlreadyQueued = true);
void PlaySound(int soundID);
bool IsPlayerInFirstPerson();
void UpdateNode(RE::NiNode* node,
                SKSE::stl::enumeration<RE::NiUpdateData::Flag, std::uint32_t> flags = RE::NiUpdateData::Flag::kNone,
                float updateTime = 0.f);
void ScaleNode(RE::Actor* a_actor);
void CloseMenu();
void SendAnimationEvent(RE::Actor* a_actor, const char* AnimationString);
void HideWeapon(RE::Actor* a_actor, bool a_cull);
void NotifyActorDrinkingLastPotion(RE::Actor* currentActor, RE::TESBoundObject* a_object = nullptr);
void LogActorData(RE::Actor* a_actor);
void EquipPotion(RE::Actor* a_actor, RE::ActorEquipManager* a_self, RE::TESBoundObject* a_object, std::uint64_t a_unk);
void AnimObjectReplaceModelByFormID(RE::FormID a_formID, const char* newModelPath);
int GetAnimationVariableInt(RE::Actor* a_actor, const std::string& variableName);
void SetAnimationVariableInt(RE::Actor* a_actor, const char* variableName, int value);
bool GetAnimationVariableBool(RE::Actor* a_actor, const std::string& variableName);
void SetAnimationVariableBool(RE::Actor* a_actor, const char* variableName, bool value);
bool CheckKeyword(RE::FormID a_formID, const std::variant<RE::TESBoundObject*, RE::TESRace*>& a_form);
bool GetSwimming(RE::Actor* a_actor);
bool CheckActorAndObjectKeywords(RE::TESBoundObject* a_object, RE::Actor* a_actor);
bool IsObjectAPotion(RE::TESBoundObject* a_object);
bool IsActorReadyForAnimation(RE::Actor* a_actor);
bool DoesFileExist(const std::string& filePath);
bool IsObjectWhitelisted(RE::TESBoundObject* a_object);
bool IsObjectBlacklisted(RE::TESBoundObject* a_object);
std::vector<std::string> GetMagicEffectNames(RE::TESBoundObject* a_object);
std::string AlchObjectGetModelPath(RE::TESBoundObject* a_object);
std::string ConvertToAbsolutePath(const std::string& relativePath);
std::string GetGameDataPath();
RE::TESObjectWEAP* CheckActorWeaponEquipped(RE::Actor* a_actor);