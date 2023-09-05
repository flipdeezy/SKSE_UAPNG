#include "actor_data.h"
#include "animation_event_sink.h"
#include "config.h"
#include "function_library.h"
#include "logger.h"
#include <filesystem>
#include <string>
#include <vector>

void SendDebugNotification(const char* message, const char* soundToPlay, bool cancelIfAlreadyQueued) {
    RE::DebugNotification(message, soundToPlay, cancelIfAlreadyQueued);
}

void PlaySound(int soundID) {
    auto soundFormID = static_cast<RE::FormID>(soundID);
    auto* audio = RE::BSAudioManager::GetSingleton();
    if (audio) {
        audio->Play(soundFormID);
    }
}

bool IsPlayerInFirstPerson() {
    auto* playerCamera = RE::PlayerCamera::GetSingleton();
    if (playerCamera) {
        return playerCamera->IsInFirstPerson();
    }
    return false;
}

void UpdateNode(RE::NiNode* node, SKSE::stl::enumeration<RE::NiUpdateData::Flag, std::uint32_t> flags,
                float updateTime) {
    auto updateData = RE::NiUpdateData();
    updateData.flags = flags;
    updateData.time = updateTime;
    node->Update(updateData);
}

void ScaleNode(RE::Actor* a_actor) {
    // Access the First-Person Node directly using Get3D
    RE::NiNode* firstpersonNode = nullptr;
    auto root3D = a_actor->Get3D(1);
    if (root3D) {
        firstpersonNode = root3D->AsNode();
    }

    if (!firstpersonNode) {
        logger::debug("Failed to find the first-person node.");
        return;
    }

    // Access the Arm Nodes
    RE::NiAVObject* leftArmAVObj = firstpersonNode->GetObjectByName("NPC L UpperArm [LUar]");
    RE::NiAVObject* rightArmAVObj = firstpersonNode->GetObjectByName("NPC R UpperArm [RUar]");

    RE::NiNode* leftarmNode = leftArmAVObj ? leftArmAVObj->AsNode() : nullptr;
    RE::NiNode* rightarmNode = rightArmAVObj ? rightArmAVObj->AsNode() : nullptr;

    // Check if the arm nodes exist
    if (!leftarmNode || !rightarmNode) {
        logger::debug("Failed to find the arm nodes.");
        return;
    }

    logger::debug("Successfully found the arm nodes.");

    // Adjust the Scale
    leftarmNode->local.scale = 1.0f;  // 0.001f to hide
    rightarmNode->local.scale = 1.0f;
    UpdateNode(leftarmNode);
    UpdateNode(rightarmNode);
}

void CloseMenu() {
    auto* uiManager = RE::UI::GetSingleton();

    auto inventoryMenu = uiManager->GetMenu<RE::InventoryMenu>();
    if (inventoryMenu) {
        RE::UIMessageQueue::GetSingleton()->AddMessage("InventoryMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
        RE::UIMessageQueue::GetSingleton()->AddMessage("TweenMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }

    auto favoritesMenu = uiManager->GetMenu<RE::FavoritesMenu>();
    if (favoritesMenu) {
        RE::UIMessageQueue::GetSingleton()->AddMessage("FavoritesMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }

    auto containerMenu = uiManager->GetMenu<RE::ContainerMenu>();
    if (containerMenu) {
        RE::UIMessageQueue::GetSingleton()->AddMessage("ContainerMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
    }
}

std::vector<std::string> GetMagicEffectNames(RE::TESBoundObject* a_object) {
    std::vector<std::string> effectNames;
    auto* alchemyItem = static_cast<RE::AlchemyItem*>(a_object);
    RE::BSTArray<RE::Effect*> effects = alchemyItem->effects;
    for (auto* effect : effects) {
        if (effect) {
            effectNames.emplace_back(effect->baseEffect->GetFullName());
        }
    }
    return effectNames;
}

void SendAnimationEvent(RE::Actor* a_actor, const char* AnimationString) {
    auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor);
    if (animGraphHolder) {
        animGraphHolder->NotifyAnimationGraph(AnimationString);
        logger::debug("Ultimate Potion Animation NG - Sending animation event '{}' to actor '{}'", AnimationString,
                      a_actor->GetName());
    } else {
        logger::error("Ultimate Potion Animation NG - Failed to get animGraphHolder for actor '{}'",
                      a_actor->GetName());
    }
}

int GetAnimationVariableInt(RE::Actor* a_actor, const std::string& variableName) {
    int value = 0;
    auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor);
    if (animGraphHolder) {
        animGraphHolder->GetGraphVariableInt(variableName.c_str(), value);
        logger::info("Ultimate Potion Animation NG - Checked variable '{}' for actor '{}': Value = {}", variableName,
                     a_actor->GetName(), value);
    } else {
        logger::error("Failed to get animGraphHolder for actor '{}'", a_actor->GetName());
    }
    return value;
}

bool GetAnimationVariableBool(RE::Actor* a_actor, const std::string& variableName) {
    bool value = false;
    auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor);
    if (animGraphHolder) {
        animGraphHolder->GetGraphVariableBool(variableName.c_str(), value);
        logger::info("Ultimate Potion Animation NG - Checked variable '{}' for actor '{}': Value = {}", variableName,
                     a_actor->GetName(), value ? "true" : "false");
    } else {
        logger::error("Ultimate Potion Animation NG - Failed to get animGraphHolder for actor '{}'",
                      a_actor->GetName());
    }
    return value;
}

void SetAnimationVariableBool(RE::Actor* a_actor, const char* variableName, bool value) {
    auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor);
    if (animGraphHolder) {
        animGraphHolder->SetGraphVariableBool(variableName, value);
        logger::debug("Ultimate Potion Animation NG - Setting animation variable '{}' to '{}' for actor '{}'",
                      variableName, value ? "true" : "false", a_actor->GetName());
    } else {
        logger::error("Ultimate Potion Animation NG - Failed to get animGraphHolder for actor '{}'",
                      a_actor->GetName());
    }
}

void HideWeapon(RE::Actor* a_actor, bool a_cull) {
    auto rootNode = a_actor->GetNodeByName("Weapon");
    if (rootNode) {
        rootNode->CullNode(a_cull);
    }
}

RE::TESObjectWEAP* CheckActorWeaponEquipped(RE::Actor* a_actor) {
    RE::TESForm* equippedObject = a_actor->GetEquippedObject(false);
    if (equippedObject && equippedObject->Is(RE::FormType::Weapon)) {
        RE::TESObjectWEAP* weapon = static_cast<RE::TESObjectWEAP*>(equippedObject);
        logger::debug("Ultimate Potion Animation NG - equippedObject: {}", weapon->GetName());
        return weapon;
    }
    return nullptr;
}

bool CheckKeyword(RE::FormID a_formID, const std::variant<RE::TESBoundObject*, RE::TESRace*>& a_form) {
    RE::TESForm* form = RE::BGSKeyword::LookupByID(a_formID);
    if (form) {
        RE::BGSKeyword* myKeyword = static_cast<RE::BGSKeyword*>(form);
        std::vector<RE::BGSKeyword*> keywords = {myKeyword};

        if (auto object = std::get_if<RE::TESBoundObject*>(&a_form)) {
            return (*object)->HasKeywordInArray(keywords, false);
        } else if (auto actor = std::get_if<RE::TESRace*>(&a_form)) {
            return (*actor)->HasKeywordInArray(keywords, false);
        }
    }
    return false;
}

bool GetSwimming(RE::Actor* a_actor) {
    if (a_actor->AsActorState()->IsSwimming()) {
        return true;
    } else {
        return false;
    }
}

bool CheckActorAndObjectKeywords(RE::TESBoundObject* a_object, RE::Actor* a_actor) {
    return CheckKeyword(0x08CDEC, a_object) && CheckKeyword(0x013794, a_actor->GetActorBase()->GetRace());
}

bool IsObjectAPotion(RE::TESBoundObject* a_object) {
    std::string objectName = a_object->GetName();
    std::transform(objectName.begin(), objectName.end(), objectName.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return objectName.find("potion") != std::string::npos;
}

bool IsActorReadyForAnimation(RE::Actor* a_actor) {
    int currentDefaultState = GetAnimationVariableInt(a_actor, "currentDefaultState");
    bool isPaired = GetAnimationVariableBool(a_actor, "bIsSynced");
    return (actorDataMap.find(a_actor) == actorDataMap.end() || !actorDataMap[a_actor].bAnimationInProgress) &&
           (currentDefaultState < 10 || currentDefaultState > 20) && !isPaired && !a_actor->IsOnMount() &&
           !a_actor->IsInMidair() && !GetSwimming(a_actor);
}

static RE::TESBoundObject* lastEquippedObject = nullptr;

void NotifyActorDrinkingLastPotion(RE::Actor* currentActor) {
    if (config::bEnableNotifications && currentActor->GetActorBase()->IsPlayer() && lastEquippedObject) {
        std::string message = "";
        message += currentActor->GetName();
        message += " is currently drinking ";
        message += lastEquippedObject->GetName();
        SendDebugNotification(message.c_str());
    }
}

void LogActorData(RE::Actor* a_actor) {
    if (actorDataMap.find(a_actor) != actorDataMap.end()) {
        ActorData& data = actorDataMap[a_actor];
        logger::debug("Ultimate Potion Animation NG - lastActor: {}", data.lastActor->GetName());
        logger::debug("Ultimate Potion Animation NG - lastObject: {}", data.lastObject->GetName());
        logger::debug("Ultimate Potion Animation NG - bDrinking: {}", data.bDrinking);
        logger::debug("Ultimate Potion Animation NG - bAnimationInProgress: {}", data.bAnimationInProgress);
    }
}

void EquipPotion(RE::Actor* a_actor, RE::ActorEquipManager* a_self, RE::TESBoundObject* a_object, std::uint64_t a_unk) {
    logger::debug("Ultimate Potion Animation NG - Equipping potion for actor '{}'", a_actor->GetName());
    RE::AlchemyItem* a_potion = a_object->As<RE::AlchemyItem>();
    if (a_potion) {
        AnimationEventSink* eventSink = GetOrCreateEventSink(a_actor, a_self, a_object, a_unk);
        if (a_actor->GetActorBase()->IsPlayer()) {
            PlaySound(0x03C7BA);
        }
        a_actor->AddAnimationGraphEventSink(eventSink);
        SendAnimationEvent(eventSink->a_actor, "HeadtrackingOff");
        SetAnimationVariableBool(eventSink->a_actor, "tdmHeadtrackingBehavior", false);

        switch (config::iMovementType) {
            case 0:
                SendAnimationEvent(a_actor, "DrinkPotionStart");
                break;
            case 1:
                SendAnimationEvent(a_actor, "DrinkPotion_Slow");
                break;
            case 2:
                SendAnimationEvent(a_actor, "DrinkPotion_NoMove");
                break;
            default:
                logger::error("Invalid movement type value: {}", config::iMovementType);
                break;
        }
        HideWeapon(a_actor, true);
    } else {
        logger::error("Ultimate Potion Animation NG - Failed to cast TESBoundObject to AlchemyItem for actor '{}'",
                      a_actor->GetName());
    }
}

std::string AlchObjectGetModelPath(RE::TESBoundObject* a_object) {
    if (a_object) {
        RE::AlchemyItem* alchItem = a_object->As<RE::AlchemyItem>();
        if (alchItem) {
            RE::TESModel* model = static_cast<RE::TESModel*>(alchItem);
            if (model) {
                std::string modelPath = model->GetModel();
                std::size_t found = modelPath.find_last_of("/\\");
                if (found != std::string::npos) {
                    modelPath = modelPath.substr(found + 1);
                }
                return "AnimObjects\\" + modelPath;
            }
        }
    }
    return "null";
}

std::string GetGameDataPath() { return "Data\\meshes\\"; }

std::string ConvertToAbsolutePath(const std::string& relativePath) {
    std::string gameDataPath = GetGameDataPath();
    return gameDataPath + relativePath;
}

bool DoesFileExist(const std::string& filePath) { return std::filesystem::exists(filePath); }

void AnimObjectReplaceModelByFormID(RE::FormID a_formID, const char* newModelPath) {
    auto* form = RE::TESObjectANIO::LookupByID(a_formID);
    if (form) {
        RE::TESObjectANIO* animObject = static_cast<RE::TESObjectANIO*>(form);
        RE::TESModel* model = static_cast<RE::TESModel*>(animObject);
        if (model) {
            model->SetModel(newModelPath);
            const char* modelPath = model->GetModel();
            std::string absoluteModelPath = ConvertToAbsolutePath(modelPath);
            if (!DoesFileExist(absoluteModelPath)) {
                logger::debug("Updated AnimObject model path does not exist: {}", absoluteModelPath);
                model->SetModel("Animobjects\\AnimObjectDrinkPotion.nif");
            }
        }
    }
}